# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

target_include_directories(mbed-cellular
    INTERFACE
        .
)

target_sources(mbed-cellular
    INTERFACE
        QUECTEL_EC2X.cpp
        QUECTEL_EC2X_CellularNetwork.cpp
        QUECTEL_EC2X_CellularContext.cpp
)
