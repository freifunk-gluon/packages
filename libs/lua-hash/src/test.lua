#!/usr/bin/lua

-- SPDX-FileCopyrightText: 2016-2021 Leonardo Mörlein <git@irrelefant.net>
-- SPDX-License-Identifier: MIT License

local hash = require "hash"
assert(hash.md5('testing') == 'ae2b1fca515949e5d54fb22b8ed95575')
