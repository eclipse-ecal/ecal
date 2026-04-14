/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include "tracing_test_helpers.h"

using namespace eCAL::tracing;

// ============ Tests for Layer Type Enum Values and Conversion ============

class TracingLayerTypeTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(TracingLayerTypeTest, LayerTypeValues)
{
    EXPECT_EQ(tl_trace_none, 0);
    EXPECT_EQ(tl_trace_shm, 1);
    EXPECT_EQ(tl_trace_udp, 2);
    EXPECT_EQ(tl_trace_tcp, 4);
    EXPECT_EQ(tl_trace_shm_udp, 3);    // 1 | 2
    EXPECT_EQ(tl_trace_shm_tcp, 5);    // 1 | 4
    EXPECT_EQ(tl_trace_udp_tcp, 6);    // 2 | 4
    EXPECT_EQ(tl_trace_all, 7);        // 1 | 2 | 4
}

TEST_F(TracingLayerTypeTest, LayerTypeBitwise)
{
    uint64_t combined = tl_trace_shm | tl_trace_udp;
    EXPECT_EQ(combined, tl_trace_shm_udp);
    EXPECT_EQ(combined, 3);

    combined = tl_trace_shm | tl_trace_tcp;
    EXPECT_EQ(combined, tl_trace_shm_tcp);
    EXPECT_EQ(combined, 5);

    combined = tl_trace_udp | tl_trace_tcp;
    EXPECT_EQ(combined, tl_trace_udp_tcp);
    EXPECT_EQ(combined, 6);

    combined = tl_trace_shm | tl_trace_udp | tl_trace_tcp;
    EXPECT_EQ(combined, tl_trace_all);
    EXPECT_EQ(combined, 7);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeSHM)
{
    eTracingLayerType result = toTracingLayerType(eCAL::tl_ecal_shm);
    EXPECT_EQ(result, tl_trace_shm);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeUDP)
{
    eTracingLayerType result = toTracingLayerType(eCAL::tl_ecal_udp);
    EXPECT_EQ(result, tl_trace_udp);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeTCP)
{
    eTracingLayerType result = toTracingLayerType(eCAL::tl_ecal_tcp);
    EXPECT_EQ(result, tl_trace_tcp);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeAll)
{
    eTracingLayerType result = toTracingLayerType(eCAL::tl_all);
    EXPECT_EQ(result, tl_trace_all);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeUnknown)
{
    eTracingLayerType result = toTracingLayerType(static_cast<eCAL::eTLayerType>(999));
    EXPECT_EQ(result, tl_trace_none);
}

TEST_F(TracingLayerTypeTest, ToTracingLayerTypeNone)
{
    eTracingLayerType result = toTracingLayerType(eCAL::tl_none);
    EXPECT_EQ(result, tl_trace_none);
}
