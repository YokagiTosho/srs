package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type SrsCommonResponse struct {
	Code int         `json:"code"`
	Data interface{} `json:"data"`
}

func SrsWriteErrorResponse(w http.ResponseWriter, err error) {
	w.WriteHeader(http.StatusInternalServerError)
	w.Write([]byte(err.Error()))
}

func SrsWriteDataResponse(w http.ResponseWriter, data interface{}) {
	j, err := json.Marshal(data)
	if err != nil {
		SrsWriteErrorResponse(w, fmt.Errorf("marshal %v, err %v", err))
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.Write(j)
}

var publishKey string
var playKey string

func splitParams(params string) []string {
	return strings.Split(params, "&")
}

func findSecret(params []string) (string, error) {
	for _, param := range params {
		r := strings.Split(param, "=")
		key := r[0]
		value := r[1]

		if key == "secret" {
			return value, nil
		}
	}

	return "", fmt.Errorf("secret not present")
}

func authPeer(rowParams string, token string) error {
	if rowParams[0] != '?' {
		return fmt.Errorf("no params specified in URL")
	}

	// get all params from URL
	params := splitParams(rowParams[1:])

	secret, err := findSecret(params)
	if err != nil {
		return err
	}

	if secret != token {
		return fmt.Errorf("not authorized")
	}

	return nil
}

var StaticDir string

// SrsCommonRequest is the common fields of request messages from SRS HTTP callback.
type SrsCommonRequest struct {
	Action   string `json:"action"`
	ClientId string `json:"client_id"`
	Ip       string `json:"ip"`
	Vhost    string `json:"vhost"`
	App      string `json:"app"`
}

func (v *SrsCommonRequest) String() string {
	return fmt.Sprintf("action=%v, client_id=%v, ip=%v, vhost=%v", v.Action, v.ClientId, v.Ip, v.Vhost)
}

/*
handle the clients requests: connect/disconnect vhost/app.
for SRS hook: on_connect/on_close
on_connect:

	when client connect to vhost/app, call the hook,
	the request in the POST data string is a object encode by json:
		  {
			  "action": "on_connect",
			  "client_id": "9308h583",
			  "ip": "192.168.1.10",
			  "vhost": "video.test.com",
			  "app": "live",
			  "tcUrl": "rtmp://video.test.com/live?key=d2fa801d08e3f90ed1e1670e6e52651a",
			  "pageUrl": "http://www.test.com/live.html"
		  }

on_close:

	when client close/disconnect to vhost/app/stream, call the hook,
	the request in the POST data string is a object encode by json:
		  {
			  "action": "on_close",
			  "client_id": "9308h583",
			  "ip": "192.168.1.10",
			  "vhost": "video.test.com",
			  "app": "live",
			  "send_bytes": 10240,
			  "recv_bytes": 10240
		  }

if valid, the hook must return HTTP code 200(Stauts OK) and response
an int value specifies the error code(0 corresponding to success):

	0
*/
type SrsClientRequest struct {
	SrsCommonRequest
	// For on_connect message
	TcUrl   string `json:"tcUrl"`
	PageUrl string `json:"pageUrl"`
	// For on_close message
	SendBytes int64 `json:"send_bytes"`
	RecvBytes int64 `json:"recv_bytes"`
}

func (v *SrsClientRequest) IsOnConnect() bool {
	return v.Action == "on_connect"
}

func (v *SrsClientRequest) IsOnClose() bool {
	return v.Action == "on_close"
}

func (v *SrsClientRequest) String() string {
	var sb strings.Builder
	sb.WriteString(v.SrsCommonRequest.String())
	if v.IsOnConnect() {
		sb.WriteString(fmt.Sprintf(", tcUrl=%v, pageUrl=%v", v.TcUrl, v.PageUrl))
	} else if v.IsOnClose() {
		sb.WriteString(fmt.Sprintf(", send_bytes=%v, recv_bytes=%v", v.SendBytes, v.RecvBytes))
	}
	return sb.String()
}

/*
for SRS hook: on_publish/on_unpublish
on_publish:

	   when client(encoder) publish to vhost/app/stream, call the hook,
	   the request in the POST data string is a object encode by json:
			 {
				 "action": "on_publish",
				 "client_id": "9308h583",
				 "ip": "192.168.1.10",
				 "vhost": "video.test.com",
				 "app": "live",
				 "stream": "livestream",
				 "param":"?token=xxx&salt=yyy"
			 }

on_unpublish:

	   when client(encoder) stop publish to vhost/app/stream, call the hook,
	   the request in the POST data string is a object encode by json:
			 {
				 "action": "on_unpublish",
				 "client_id": "9308h583",
				 "ip": "192.168.1.10",
				 "vhost": "video.test.com",
				 "app": "live",
				 "stream": "livestream",
				 "param":"?token=xxx&salt=yyy"
			 }

if valid, the hook must return HTTP code 200(Stauts OK) and response
an int value specifies the error code(0 corresponding to success):

	0
*/
type SrsStreamRequest struct {
	SrsCommonRequest
	Stream string `json:"stream"`
	Param  string `json:"param"`
}

func (v *SrsStreamRequest) String() string {
	var sb strings.Builder
	sb.WriteString(v.SrsCommonRequest.String())
	if v.IsOnPublish() || v.IsOnUnPublish() {
		sb.WriteString(fmt.Sprintf(", stream=%v, param=%v", v.Stream, v.Param))
	}
	return sb.String()
}

func (v *SrsStreamRequest) IsOnPublish() bool {
	return v.Action == "on_publish"
}

func (v *SrsStreamRequest) IsOnUnPublish() bool {
	return v.Action == "on_unpublish"
}

/*
for SRS hook: on_play/on_stop
on_play:
   when client(encoder) publish to vhost/app/stream, call the hook,
   the request in the POST data string is a object encode by json:
		 {
			 "action": "on_play",
			 "client_id": "9308h583",
			 "ip": "192.168.1.10",
			 "vhost": "video.test.com",
			 "app": "live",
			 "stream": "livestream",
			 "param":"?token=xxx&salt=yyy",
			 "pageUrl": "http://www.test.com/live.html"
		 }
on_stop:
   when client(encoder) stop publish to vhost/app/stream, call the hook,
   the request in the POST data string is a object encode by json:
		 {
			 "action": "on_stop",
			 "client_id": "9308h583",
			 "ip": "192.168.1.10",
			 "vhost": "video.test.com",
			 "app": "live",
			 "stream": "livestream",
			 "param":"?token=xxx&salt=yyy"
		 }
if valid, the hook must return HTTP code 200(Stauts OK) and response
an int value specifies the error code(0 corresponding to success):
	 0
*/

type SrsSessionRequest struct {
	SrsCommonRequest
	Stream string `json:"stream"`
	Param  string `json:"param"`
	// For on_play only.
	PageUrl string `json:"pageUrl"`
}

func (v *SrsSessionRequest) String() string {
	var sb strings.Builder
	sb.WriteString(v.SrsCommonRequest.String())
	if v.IsOnPlay() || v.IsOnStop() {
		sb.WriteString(fmt.Sprintf(", stream=%v, param=%v", v.Stream, v.Param))
	}
	if v.IsOnPlay() {
		sb.WriteString(fmt.Sprintf(", pageUrl=%v", v.PageUrl))
	}
	return sb.String()
}

func (v *SrsSessionRequest) IsOnPlay() bool {
	return v.Action == "on_play"
}

func (v *SrsSessionRequest) IsOnStop() bool {
	return v.Action == "on_stop"
}

func main() {
	// get environment variables for private keys
	publishKey = os.Getenv("PUBLISH_KEY")
	if publishKey == "" {
		log.Fatalf("PUBLISH_KEY is not set")
	}
	log.Println("PUBLISH_KEY is set:", publishKey)
	// if playKey is not set, then stream will be played without authentication
	playKey = os.Getenv("PLAY_KEY")
	if playKey != "" {
		log.Println("PLAY_KEY is set:", playKey)
	}

	srsBin := os.Args[0]
	if strings.HasPrefix(srsBin, "/var") {
		srsBin = "go run ."
	}

	var port int
	var ffmpegPath string
	flag.IntVar(&port, "p", 8085, "HTTP listen port. Default is 8085")
	flag.StringVar(&StaticDir, "s", "./static-dir", "HTML home for snapshot. Default is ./static-dir")
	flag.StringVar(&ffmpegPath, "ffmpeg", "/usr/local/bin/ffmpeg", "FFmpeg for snapshot. Default is /usr/local/bin/ffmpeg")
	flag.Usage = func() {
		fmt.Println("A demo api-server for SRS\n")
		fmt.Println(fmt.Sprintf("Usage: %v [flags]", srsBin))
		flag.PrintDefaults()
		fmt.Println(fmt.Sprintf("For example:"))
		fmt.Println(fmt.Sprintf(" 		%v -p 8085", srsBin))
		fmt.Println(fmt.Sprintf(" 		%v 8085", srsBin))
	}
	flag.Parse()

	log.SetFlags(log.Lshortfile | log.Ldate | log.Ltime | log.Lmicroseconds)

	// check if only one number arg
	if len(os.Args[1:]) == 1 {
		portArg := os.Args[1]
		var err error
		if port, err = strconv.Atoi(portArg); err != nil {
			log.Println(fmt.Sprintf("parse port arg:%v to int failed, err %v", portArg, err))
			flag.Usage()
			os.Exit(1)
		}
	}

	StaticDir, err := filepath.Abs(StaticDir)
	if err != nil {
		panic(err)
	}
	log.Println(fmt.Sprintf("api server listen at port:%v, static_dir:%v", port, StaticDir))

	http.Handle("/", http.FileServer(http.Dir(StaticDir)))
	http.HandleFunc("/api/v1", func(writer http.ResponseWriter, request *http.Request) {
		res := &struct {
			Code int `json:"code"`
			Urls struct {
				Clients  string `json:"clients"`
				Streams  string `json:"streams"`
				Sessions string `json:"sessions"`
				Chats    string `json:"chats"`
				Servers  struct {
					Summary string `json:"summary"`
					Get     string `json:"GET"`
					Post    string `json:"POST ip=node_ip&device_id=device_id"`
				}
			} `json:"urls"`
		}{
			Code: 0,
		}
		res.Urls.Clients = "for srs http callback, to handle the clients requests: connect/disconnect vhost/app."
		res.Urls.Streams = "for srs http callback, to handle the streams requests: publish/unpublish stream."
		res.Urls.Sessions = "for srs http callback, to handle the sessions requests: client play/stop stream."
		//res.Urls.Chats = "for srs demo meeting, the chat streams, public chat room."
		res.Urls.Servers.Summary = "for srs raspberry-pi and meeting demo."
		res.Urls.Servers.Get = "get the current raspberry-pi servers info."
		res.Urls.Servers.Post = "the new raspberry-pi server info."
		// TODO: no snapshots
		body, _ := json.Marshal(res)
		writer.Write(body)
	})

	// handle the clients requests: connect/disconnect vhost/app.
	http.HandleFunc("/api/v1/clients", func(w http.ResponseWriter, r *http.Request) {
		if r.Method != "POST" {
			SrsWriteDataResponse(w, struct{}{})
			return
		}

		if err := func() error {
			body, err := io.ReadAll(io.Reader(r.Body))
			if err != nil {
				return fmt.Errorf("read request body, err %v", err)
			}
			log.Println(fmt.Sprintf("post to clients, req=%v", string(body)))

			msg := &SrsClientRequest{}
			if err := json.Unmarshal(body, msg); err != nil {
				return fmt.Errorf("parse message from %v, err %v", string(body), err)
			}
			log.Println(fmt.Sprintf("Got %v", msg.String()))

			if !msg.IsOnConnect() && !msg.IsOnClose() {
				return fmt.Errorf("invalid message %v", msg.String())
			}

			SrsWriteDataResponse(w, &SrsCommonResponse{Code: 0})
			return nil
		}(); err != nil {
			SrsWriteErrorResponse(w, err)
		}
	})

	// handle the streams requests: publish/unpublish stream.
	http.HandleFunc("/api/v1/streams", func(w http.ResponseWriter, r *http.Request) {
		if r.Method != "POST" {
			SrsWriteDataResponse(w, struct{}{})
			return
		}

		if err := func() error {
			body, err := io.ReadAll(io.Reader(r.Body))
			if err != nil {
				return fmt.Errorf("read request body, err %v", err)
			}
			log.Println(fmt.Sprintf("post to streams, req=%v", string(body)))

			msg := &SrsStreamRequest{}
			if err := json.Unmarshal(body, msg); err != nil {
				return fmt.Errorf("parse message from %v, err %v", string(body), err)
			}
			log.Println(fmt.Sprintf("Got %v", msg.String()))

			if !msg.IsOnPublish() && !msg.IsOnUnPublish() {
				return fmt.Errorf("invalid message %v", msg.String())
			}

			if msg.IsOnPublish() {
				// for publishing param must be specified
				if len(msg.Param) == 0 {
					return fmt.Errorf("no param is specified")
				}

				err := authPeer(msg.Param, publishKey)
				if err != nil {
					return err
				}
			}

			SrsWriteDataResponse(w, &SrsCommonResponse{Code: 0})
			return nil
		}(); err != nil {
			SrsWriteErrorResponse(w, err)
		}
	})

	// handle the sessions requests: client play/stop stream
	http.HandleFunc("/api/v1/sessions", func(w http.ResponseWriter, r *http.Request) {
		if r.Method != "POST" {
			SrsWriteDataResponse(w, struct{}{})
			return
		}

		if err := func() error {
			body, err := io.ReadAll(io.Reader(r.Body))
			if err != nil {
				return fmt.Errorf("read request body, err %v", err)
			}
			log.Println(fmt.Sprintf("post to sessions, req=%v", string(body)))

			msg := &SrsSessionRequest{}
			if err := json.Unmarshal(body, msg); err != nil {
				return fmt.Errorf("parse message from %v, err %v", string(body), err)
			}
			log.Println(fmt.Sprintf("Got %v", msg.String()))

			if !msg.IsOnPlay() && !msg.IsOnStop() {
				return fmt.Errorf("invalid message %v", msg.String())
			}

			if msg.IsOnPlay() {
				// if playKey was set, then match against it
				// otherwise just ignore
				if playKey != "" {
					if len(msg.Param) == 0 {
						return fmt.Errorf("private key was specified for playing, but param is not set")
					}

					err := authPeer(msg.Param, playKey)
					if err != nil {
						return err
					}
				}
			}

			SrsWriteDataResponse(w, &SrsCommonResponse{Code: 0})
			return nil
		}(); err != nil {
			SrsWriteErrorResponse(w, err)
		}
	})

	// not support yet
	http.HandleFunc("/api/v1/chat", func(w http.ResponseWriter, r *http.Request) {
		SrsWriteErrorResponse(w, fmt.Errorf("not implemented"))
	})

	addr := fmt.Sprintf(":%v", port)
	log.Println(fmt.Sprintf("start listen on:%v", addr))
	if err := http.ListenAndServe(addr, nil); err != nil {
		log.Println(fmt.Sprintf("listen on addr:%v failed, err is %v", addr, err))
	}
}
