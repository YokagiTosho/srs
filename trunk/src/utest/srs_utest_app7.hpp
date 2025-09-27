//
// Copyright (c) 2013-2025 The SRS Authors
//
// SPDX-License-Identifier: MIT
//

#ifndef SRS_UTEST_APP7_HPP
#define SRS_UTEST_APP7_HPP

/*
#include <srs_utest_app7.hpp>
*/
#include <srs_utest.hpp>

#include <srs_app_rtc_conn.hpp>
#include <srs_app_rtc_source.hpp>
#include <srs_kernel_rtc_queue.hpp>
#include <srs_utest_app6.hpp>
#include <srs_utest_app2.hpp>
#include <srs_utest_service.hpp>
#include <srs_utest_kernel3.hpp>

// Mock video recv track for testing check_send_nacks
class MockRtcVideoRecvTrackForNack : public SrsRtcVideoRecvTrack
{
public:
    srs_error_t check_send_nacks_error_;
    int check_send_nacks_count_;

public:
    MockRtcVideoRecvTrackForNack(ISrsRtcPacketReceiver *receiver, SrsRtcTrackDescription *track_desc);
    virtual ~MockRtcVideoRecvTrackForNack();

public:
    virtual srs_error_t check_send_nacks();
    void set_check_send_nacks_error(srs_error_t err);
    void reset();
};

// Mock audio recv track for testing check_send_nacks
class MockRtcAudioRecvTrackForNack : public SrsRtcAudioRecvTrack
{
public:
    srs_error_t check_send_nacks_error_;
    int check_send_nacks_count_;

public:
    MockRtcAudioRecvTrackForNack(ISrsRtcPacketReceiver *receiver, SrsRtcTrackDescription *track_desc);
    virtual ~MockRtcAudioRecvTrackForNack();

public:
    virtual srs_error_t check_send_nacks();
    void set_check_send_nacks_error(srs_error_t err);
    void reset();
};

// Mock NACK timer handler for testing SrsRtcConnectionNackTimer
class MockRtcConnectionNackTimerHandler : public ISrsRtcConnectionNackTimerHandler
{
public:
    srs_error_t do_check_send_nacks_error_;
    int do_check_send_nacks_count_;

public:
    MockRtcConnectionNackTimerHandler();
    virtual ~MockRtcConnectionNackTimerHandler();

public:
    virtual srs_error_t do_check_send_nacks();
    void set_do_check_send_nacks_error(srs_error_t err);
    void reset();
};

// Mock RTC connection for testing on_before_dispose
class MockRtcConnectionForDispose : public ISrsResource
{
public:
    SrsContextId cid_;
    std::string desc_;
    bool disposing_;

public:
    MockRtcConnectionForDispose();
    virtual ~MockRtcConnectionForDispose();

public:
    virtual const SrsContextId &get_id();
    virtual std::string desc();
    void set_disposing(bool disposing);
};

// Mock connection manager for testing expire functionality
class MockConnectionManagerForExpire : public ISrsResourceManager
{
public:
    ISrsResource *removed_resource_;
    int remove_count_;

public:
    MockConnectionManagerForExpire();
    virtual ~MockConnectionManagerForExpire();

public:
    virtual void remove(ISrsResource *c);
    virtual void subscribe(ISrsDisposingHandler *h);
    virtual void unsubscribe(ISrsDisposingHandler *h);
    void reset();
};

// Mock NACK receiver for testing check_send_nacks
class MockRtpNackForReceiver : public SrsRtpNackForReceiver
{
public:
    uint32_t timeout_nacks_to_return_;
    std::vector<uint16_t> nack_seqs_to_add_;
    int get_nack_seqs_count_;

public:
    MockRtpNackForReceiver(SrsRtpRingBuffer *rtp, size_t queue_size);
    virtual ~MockRtpNackForReceiver();

public:
    virtual void get_nack_seqs(SrsRtcpNack &seqs, uint32_t &timeout_nacks);
    void set_timeout_nacks(uint32_t timeout_nacks);
    void add_nack_seq(uint16_t seq);
    void reset();
};

// Mock RTC connection for testing check_send_nacks
class MockRtcConnectionForNack : public SrsRtcConnection
{
public:
    srs_error_t send_rtcp_error_;
    int send_rtcp_count_;
    std::vector<std::string> sent_rtcp_data_;

public:
    MockRtcConnectionForNack(ISrsExecRtcAsyncTask *async, const SrsContextId &cid);
    virtual ~MockRtcConnectionForNack();

public:
    virtual srs_error_t send_rtcp(char *data, int nb_data);
    void set_send_rtcp_error(srs_error_t err);
    void reset();
};



#endif
