//
// Copyright (c) 2013-2025 The SRS Authors
//
// SPDX-License-Identifier: MIT
//

#ifndef SRS_UTEST_APP6_HPP
#define SRS_UTEST_APP6_HPP

/*
#include <srs_utest_app6.hpp>
*/
#include <srs_utest.hpp>

#include <srs_app_config.hpp>
#include <srs_app_http_hooks.hpp>
#include <srs_app_rtc_conn.hpp>
#include <srs_app_rtc_dtls.hpp>
#include <srs_app_rtc_network.hpp>
#include <srs_app_rtc_source.hpp>
#include <srs_app_rtmp_source.hpp>
#include <srs_app_srt_source.hpp>
#include <srs_app_statistic.hpp>
#include <srs_protocol_sdp.hpp>
#include <srs_utest_app2.hpp>
#include <srs_utest_protocol3.hpp>

// Mock DTLS implementation for testing SrsSecurityTransport
class MockDtls : public ISrsDtls
{
public:
    srs_error_t initialize_error_;
    srs_error_t start_active_handshake_error_;
    srs_error_t on_dtls_error_;
    srs_error_t get_srtp_key_error_;
    std::string last_role_;
    std::string last_version_;
    std::string recv_key_;
    std::string send_key_;
    int initialize_count_;
    int start_active_handshake_count_;
    int on_dtls_count_;
    int get_srtp_key_count_;

public:
    MockDtls();
    virtual ~MockDtls();

public:
    virtual srs_error_t initialize(std::string role, std::string version);
    virtual srs_error_t start_active_handshake();
    virtual srs_error_t on_dtls(char *data, int nb_data);
    virtual srs_error_t get_srtp_key(std::string &recv_key, std::string &send_key);

public:
    void reset();
    void set_initialize_error(srs_error_t err);
    void set_start_active_handshake_error(srs_error_t err);
    void set_on_dtls_error(srs_error_t err);
    void set_get_srtp_key_error(srs_error_t err);
    void set_srtp_keys(const std::string &recv_key, const std::string &send_key);
};

// Mock RTC Network for testing SrsSecurityTransport
class MockRtcNetwork : public ISrsRtcNetwork
{
public:
    srs_error_t on_dtls_handshake_done_error_;
    srs_error_t on_dtls_alert_error_;
    srs_error_t protect_rtp_error_;
    srs_error_t protect_rtcp_error_;
    srs_error_t write_error_;
    std::string last_alert_type_;
    std::string last_alert_desc_;
    int on_dtls_handshake_done_count_;
    int on_dtls_alert_count_;
    int protect_rtp_count_;
    int protect_rtcp_count_;
    int write_count_;
    bool is_established_;

public:
    MockRtcNetwork();
    virtual ~MockRtcNetwork();

public:
    virtual srs_error_t on_dtls_handshake_done();
    virtual srs_error_t on_dtls_alert(std::string type, std::string desc);
    virtual srs_error_t protect_rtp(void *packet, int *nb_cipher);
    virtual srs_error_t protect_rtcp(void *packet, int *nb_cipher);
    virtual bool is_establelished();
    virtual srs_error_t write(void *buf, size_t size, ssize_t *nwrite);

public:
    void reset();
    void set_on_dtls_handshake_done_error(srs_error_t err);
    void set_on_dtls_alert_error(srs_error_t err);
    void set_protect_rtp_error(srs_error_t err);
    void set_protect_rtcp_error(srs_error_t err);
    void set_write_error(srs_error_t err);
    void set_established(bool established);
};

// Mock SRTP implementation for testing SrsSecurityTransport
class MockSrtp : public ISrsSRTP
{
public:
    srs_error_t initialize_error_;
    srs_error_t protect_rtp_error_;
    srs_error_t protect_rtcp_error_;
    srs_error_t unprotect_rtp_error_;
    srs_error_t unprotect_rtcp_error_;
    std::string last_recv_key_;
    std::string last_send_key_;
    int initialize_count_;
    int protect_rtp_count_;
    int protect_rtcp_count_;
    int unprotect_rtp_count_;
    int unprotect_rtcp_count_;

public:
    MockSrtp();
    virtual ~MockSrtp();

public:
    virtual srs_error_t initialize(std::string recv_key, std::string send_key);
    virtual srs_error_t protect_rtp(void *packet, int *nb_cipher);
    virtual srs_error_t protect_rtcp(void *packet, int *nb_cipher);
    virtual srs_error_t unprotect_rtp(void *packet, int *nb_plaintext);
    virtual srs_error_t unprotect_rtcp(void *packet, int *nb_plaintext);

public:
    void reset();
    void set_initialize_error(srs_error_t err);
    void set_protect_rtp_error(srs_error_t err);
    void set_protect_rtcp_error(srs_error_t err);
    void set_unprotect_rtp_error(srs_error_t err);
    void set_unprotect_rtcp_error(srs_error_t err);
};

// Mock PLI Worker Handler for testing SrsRtcPliWorker
class MockRtcPliWorkerHandler : public ISrsRtcPliWorkerHandler
{
public:
    srs_error_t do_request_keyframe_error_;
    std::vector<std::pair<uint32_t, SrsContextId> > keyframe_requests_;
    int do_request_keyframe_count_;

public:
    MockRtcPliWorkerHandler();
    virtual ~MockRtcPliWorkerHandler();

public:
    virtual srs_error_t do_request_keyframe(uint32_t ssrc, SrsContextId cid);

public:
    void reset();
    void set_do_request_keyframe_error(srs_error_t err);
    bool has_keyframe_request(uint32_t ssrc, const SrsContextId &cid);
    int get_keyframe_request_count();
};

// Mock PLI Worker for testing SrsRtcPlayStream::on_stream_change
class MockRtcPliWorker : public SrsRtcPliWorker
{
public:
    std::vector<std::pair<uint32_t, SrsContextId> > keyframe_requests_;
    int request_keyframe_count_;

public:
    MockRtcPliWorker(ISrsRtcPliWorkerHandler *h);
    virtual ~MockRtcPliWorker();

public:
    virtual void request_keyframe(uint32_t ssrc, SrsContextId cid);

public:
    void reset();
    bool has_keyframe_request(uint32_t ssrc, const SrsContextId &cid);
    int get_keyframe_request_count();
};

// Mock HTTP hooks for testing SrsRtcAsyncCallOnStop
class MockHttpHooks : public ISrsHttpHooks
{
public:
    std::vector<std::pair<std::string, ISrsRequest *> > on_stop_calls_;
    int on_stop_count_;
    std::vector<std::pair<std::string, ISrsRequest *> > on_unpublish_calls_;
    int on_unpublish_count_;

public:
    MockHttpHooks();
    virtual ~MockHttpHooks();
    virtual srs_error_t on_connect(std::string url, ISrsRequest *req);
    virtual void on_close(std::string url, ISrsRequest *req, int64_t send_bytes, int64_t recv_bytes);
    virtual srs_error_t on_publish(std::string url, ISrsRequest *req);
    virtual void on_unpublish(std::string url, ISrsRequest *req);
    virtual srs_error_t on_play(std::string url, ISrsRequest *req);
    virtual void on_stop(std::string url, ISrsRequest *req);
    virtual srs_error_t on_dvr(SrsContextId cid, std::string url, ISrsRequest *req, std::string file);
    virtual srs_error_t on_hls(SrsContextId cid, std::string url, ISrsRequest *req, std::string file, std::string ts_url,
                               std::string m3u8, std::string m3u8_url, int sn, srs_utime_t duration);
    virtual srs_error_t on_hls_notify(SrsContextId cid, std::string url, ISrsRequest *req, std::string ts_url, int nb_notify);
    virtual srs_error_t discover_co_workers(std::string url, std::string &host, int &port);
    virtual srs_error_t on_forward_backend(std::string url, ISrsRequest *req, std::vector<std::string> &rtmp_urls);
    void clear_calls();
};

// Mock context for testing SrsRtcAsyncCallOnStop
class MockContext : public ISrsContext
{
public:
    SrsContextId current_id_;
    SrsContextId get_id_result_;

public:
    MockContext();
    virtual ~MockContext();
    virtual SrsContextId generate_id();
    virtual const SrsContextId &get_id();
    virtual const SrsContextId &set_id(const SrsContextId &v);
    void set_current_id(const SrsContextId &id);
};

// Mock app config for testing SrsRtcAsyncCallOnStop
class MockAppConfig : public ISrsAppConfig
{
public:
    bool http_hooks_enabled_;
    SrsConfDirective *on_stop_directive_;
    SrsConfDirective *on_unpublish_directive_;
    bool rtc_nack_enabled_;
    bool rtc_nack_no_copy_;
    int rtc_drop_for_pt_;
    bool rtc_twcc_enabled_;
    bool srt_enabled_;
    bool rtc_to_rtmp_;

public:
    MockAppConfig();
    virtual ~MockAppConfig();
    // ISrsConfig methods
    virtual srs_utime_t get_pithy_print();
    virtual std::string get_default_app_name();
    // ISrsAppConfig methods
    virtual bool get_vhost_http_hooks_enabled(std::string vhost);
    virtual SrsConfDirective *get_vhost_on_stop(std::string vhost);
    virtual SrsConfDirective *get_vhost_on_unpublish(std::string vhost);
    virtual bool get_rtc_nack_enabled(std::string vhost);
    virtual bool get_rtc_nack_no_copy(std::string vhost);
    virtual bool get_realtime_enabled(std::string vhost, bool is_rtc);
    virtual int get_mw_msgs(std::string vhost, bool is_realtime, bool is_rtc);
    virtual int get_rtc_drop_for_pt(std::string vhost);
    virtual bool get_rtc_twcc_enabled(std::string vhost);
    virtual bool get_srt_enabled();
    virtual bool get_srt_enabled(std::string vhost);
    virtual bool get_rtc_to_rtmp(std::string vhost);
    virtual srs_utime_t get_rtc_stun_timeout(std::string vhost);
    virtual bool get_rtc_stun_strict_check(std::string vhost);
    void set_http_hooks_enabled(bool enabled);
    void set_on_stop_urls(const std::vector<std::string> &urls);
    void clear_on_stop_directive();
    void set_on_unpublish_urls(const std::vector<std::string> &urls);
    void clear_on_unpublish_directive();
    void set_rtc_nack_enabled(bool enabled);
    void set_rtc_nack_no_copy(bool no_copy);
    void set_rtc_drop_for_pt(int pt);
    void set_rtc_twcc_enabled(bool enabled);
    void set_srt_enabled(bool enabled);
    void set_rtc_to_rtmp(bool enabled);
};

// Mock request for testing SrsRtcAsyncCallOnStop
class MockRtcAsyncCallRequest : public ISrsRequest
{
public:
    std::string vhost_;
    std::string app_;
    std::string stream_;

public:
    MockRtcAsyncCallRequest(std::string vhost = "__defaultVhost__", std::string app = "live", std::string stream = "test");
    virtual ~MockRtcAsyncCallRequest();
    virtual ISrsRequest *copy();
    virtual std::string get_stream_url();
    virtual void update_auth(ISrsRequest *req);
    virtual void strip();
    virtual ISrsRequest *as_http();
};

// Mock RTC source manager for testing SrsRtcPlayStream
class MockRtcSourceManager : public ISrsRtcSourceManager
{
public:
    srs_error_t initialize_error_;
    srs_error_t fetch_or_create_error_;
    int initialize_count_;
    int fetch_or_create_count_;
    SrsSharedPtr<SrsRtcSource> mock_source_;

public:
    MockRtcSourceManager();
    virtual ~MockRtcSourceManager();
    virtual srs_error_t initialize();
    virtual srs_error_t fetch_or_create(ISrsRequest *r, SrsSharedPtr<SrsRtcSource> &pps);
    virtual SrsSharedPtr<SrsRtcSource> fetch(ISrsRequest *r);
    void set_initialize_error(srs_error_t err);
    void set_fetch_or_create_error(srs_error_t err);
    void reset();
};

// Mock statistic for testing SrsRtcPlayStream
class MockRtcStatistic : public ISrsStatistic
{
public:
    srs_error_t on_client_error_;
    int on_client_count_;
    int on_disconnect_count_;
    std::string last_client_id_;
    ISrsRequest *last_client_req_;
    ISrsExpire *last_client_conn_;
    SrsRtmpConnType last_client_type_;

public:
    MockRtcStatistic();
    virtual ~MockRtcStatistic();
    virtual void on_disconnect(std::string id, srs_error_t err);
    virtual srs_error_t on_client(std::string id, ISrsRequest *req, ISrsExpire *conn, SrsRtmpConnType type);
    void set_on_client_error(srs_error_t err);
    void reset();
};

// Mock RTC async task executor for testing SrsRtcPlayStream::send_packet
class MockRtcAsyncTaskExecutor : public ISrsExecRtcAsyncTask
{
public:
    srs_error_t exec_error_;
    int exec_count_;
    ISrsAsyncCallTask *last_task_;

public:
    MockRtcAsyncTaskExecutor();
    virtual ~MockRtcAsyncTaskExecutor();

public:
    virtual srs_error_t exec_rtc_async_work(ISrsAsyncCallTask *t);
    void set_exec_error(srs_error_t err);
    void reset();
};

// Mock RTC packet sender for testing SrsRtcPlayStream::send_packet
class MockRtcPacketSender : public ISrsRtcPacketSender
{
public:
    srs_error_t send_packet_error_;
    int send_packet_count_;
    SrsRtpPacket *last_sent_packet_;

public:
    MockRtcPacketSender();
    virtual ~MockRtcPacketSender();

public:
    virtual srs_error_t do_send_packet(SrsRtpPacket *pkt);
    void set_send_packet_error(srs_error_t err);
    void reset();
};

// Mock RTC send track for testing SrsRtcPlayStream::send_packet
class MockRtcSendTrack : public SrsRtcSendTrack
{
public:
    srs_error_t on_rtp_error_;
    srs_error_t on_nack_error_;
    int on_rtp_count_;
    int on_nack_count_;
    SrsRtpPacket *last_rtp_packet_;
    SrsRtpPacket **last_nack_packet_;
    bool nack_set_to_null_;

public:
    MockRtcSendTrack(ISrsRtcPacketSender *sender, SrsRtcTrackDescription *track_desc, bool is_audio);
    virtual ~MockRtcSendTrack();

public:
    virtual srs_error_t on_rtp(SrsRtpPacket *pkt);
    virtual srs_error_t on_rtcp(SrsRtpPacket *pkt);
    virtual srs_error_t on_nack(SrsRtpPacket **ppkt);
    void set_on_rtp_error(srs_error_t err);
    void set_on_nack_error(srs_error_t err);
    void set_nack_set_to_null(bool v);
    void reset();
};

// Mock RTCP classes for testing SrsRtcPlayStream::on_rtcp
class MockRtcpCommon : public SrsRtcpCommon
{
public:
    uint8_t mock_type_;

public:
    MockRtcpCommon(uint8_t type);
    virtual ~MockRtcpCommon();
    virtual uint8_t type() const;
};

class MockRtcpRR : public SrsRtcpRR
{
public:
    MockRtcpRR(uint32_t sender_ssrc = 0);
    virtual ~MockRtcpRR();
};

class MockRtcpNack : public SrsRtcpNack
{
public:
    MockRtcpNack(uint32_t sender_ssrc = 0);
    virtual ~MockRtcpNack();
};

class MockRtcpFbCommon : public SrsRtcpFbCommon
{
public:
    uint8_t mock_rc_;

public:
    MockRtcpFbCommon(uint8_t rc = kPLI);
    virtual ~MockRtcpFbCommon();
    virtual uint8_t get_rc() const;
};

class MockRtcpXr : public SrsRtcpXr
{
public:
    MockRtcpXr(uint32_t ssrc = 0);
    virtual ~MockRtcpXr();
};

// Mock RTC send track with NACK response capability for testing on_rtcp_nack
class MockRtcSendTrackForNack : public SrsRtcSendTrack
{
public:
    srs_error_t on_recv_nack_error_;
    int on_recv_nack_count_;
    std::vector<uint16_t> last_lost_seqs_;
    uint32_t test_ssrc_;
    bool track_enabled_;

public:
    MockRtcSendTrackForNack(ISrsRtcPacketSender *sender, SrsRtcTrackDescription *track_desc, bool is_audio, uint32_t ssrc);
    virtual ~MockRtcSendTrackForNack();

public:
    virtual srs_error_t on_rtp(SrsRtpPacket *pkt);
    virtual srs_error_t on_rtcp(SrsRtpPacket *pkt);
    virtual srs_error_t on_recv_nack(const std::vector<uint16_t> &lost_seqs);
    virtual bool has_ssrc(uint32_t ssrc);
    virtual bool get_track_status();

public:
    void set_track_enabled(bool enabled);
    void set_on_recv_nack_error(srs_error_t err);
    void reset();
};

// Mock RTCP sender for testing SrsRtcPublishRtcpTimer
class MockRtcRtcpSender : public ISrsRtcRtcpSender
{
public:
    bool is_sender_started_;
    bool is_sender_twcc_enabled_;
    srs_error_t send_rtcp_rr_error_;
    srs_error_t send_rtcp_xr_rrtr_error_;
    srs_error_t send_periodic_twcc_error_;
    int send_rtcp_rr_count_;
    int send_rtcp_xr_rrtr_count_;
    int send_periodic_twcc_count_;

public:
    MockRtcRtcpSender();
    virtual ~MockRtcRtcpSender();

public:
    virtual bool is_sender_started();
    virtual srs_error_t send_rtcp_rr();
    virtual srs_error_t send_rtcp_xr_rrtr();
    virtual bool is_sender_twcc_enabled();
    virtual srs_error_t send_periodic_twcc();

public:
    void set_sender_started(bool started);
    void set_sender_twcc_enabled(bool enabled);
    void set_send_rtcp_rr_error(srs_error_t err);
    void set_send_rtcp_xr_rrtr_error(srs_error_t err);
    void set_send_periodic_twcc_error(srs_error_t err);
    void reset();
};

// Mock RTC packet receiver for testing SrsRtcPublishStream
class MockRtcPacketReceiver : public ISrsRtcPacketReceiver
{
public:
    srs_error_t send_rtcp_rr_error_;
    srs_error_t send_rtcp_xr_rrtr_error_;
    srs_error_t send_rtcp_error_;
    srs_error_t send_rtcp_fb_pli_error_;
    int send_rtcp_rr_count_;
    int send_rtcp_xr_rrtr_count_;
    int send_rtcp_count_;
    int send_rtcp_fb_pli_count_;
    int check_send_nacks_count_;

public:
    MockRtcPacketReceiver();
    virtual ~MockRtcPacketReceiver();

public:
    virtual srs_error_t send_rtcp_rr(uint32_t ssrc, SrsRtpRingBuffer *rtp_queue, const uint64_t &last_send_systime, const SrsNtp &last_send_ntp);
    virtual srs_error_t send_rtcp_xr_rrtr(uint32_t ssrc);
    virtual void check_send_nacks(SrsRtpNackForReceiver *nack, uint32_t ssrc, uint32_t &sent_nacks, uint32_t &timeout_nacks);
    virtual srs_error_t send_rtcp(char *data, int nb_data);
    virtual srs_error_t send_rtcp_fb_pli(uint32_t ssrc, const SrsContextId &cid_of_subscriber);

public:
    void set_send_rtcp_rr_error(srs_error_t err);
    void set_send_rtcp_xr_rrtr_error(srs_error_t err);
    void set_send_rtcp_error(srs_error_t err);
    void set_send_rtcp_fb_pli_error(srs_error_t err);
    void reset();
};

// Mock expire for testing SrsRtcPublishStream
class MockRtcExpire : public ISrsExpire
{
public:
    bool expired_;

public:
    MockRtcExpire();
    virtual ~MockRtcExpire();

public:
    virtual void expire();
    void reset();
};

// Mock live source manager for testing SrsRtcPublishStream
class MockLiveSourceManager : public ISrsLiveSourceManager
{
public:
    srs_error_t fetch_or_create_error_;
    int fetch_or_create_count_;
    SrsSharedPtr<SrsLiveSource> mock_source_;
    bool can_publish_;

public:
    MockLiveSourceManager();
    virtual ~MockLiveSourceManager();
    virtual srs_error_t fetch_or_create(ISrsRequest *r, SrsSharedPtr<SrsLiveSource> &pps);
    virtual SrsSharedPtr<SrsLiveSource> fetch(ISrsRequest *r);
    void set_fetch_or_create_error(srs_error_t err);
    void set_can_publish(bool can_publish);
    void reset();
};

// Mock live source for testing SrsRtcPublishStream
class MockLiveSource : public SrsLiveSource
{
public:
    bool can_publish_result_;

public:
    MockLiveSource();
    virtual ~MockLiveSource();
    virtual bool can_publish(bool is_edge);
    void set_can_publish(bool can_publish);
};

// Mock SRT source for testing SrsRtcPublishStream
class MockSrtSource : public SrsSrtSource
{
public:
    bool can_publish_result_;

public:
    MockSrtSource();
    virtual ~MockSrtSource();
    virtual bool can_publish();
    void set_can_publish(bool can_publish);
};

// Mock SRT source manager for testing SrsRtcPublishStream
class MockSrtSourceManager : public ISrsSrtSourceManager
{
public:
    srs_error_t initialize_error_;
    srs_error_t fetch_or_create_error_;
    int initialize_count_;
    int fetch_or_create_count_;
    SrsSharedPtr<SrsSrtSource> mock_source_;
    bool can_publish_;

public:
    MockSrtSourceManager();
    virtual ~MockSrtSourceManager();
    virtual srs_error_t initialize();
    virtual srs_error_t fetch_or_create(ISrsRequest *r, SrsSharedPtr<SrsSrtSource> &pps);
    virtual SrsSharedPtr<SrsSrtSource> fetch(ISrsRequest *r);
    void set_initialize_error(srs_error_t err);
    void set_fetch_or_create_error(srs_error_t err);
    void set_can_publish(bool can_publish);
    void reset();
};

#endif
