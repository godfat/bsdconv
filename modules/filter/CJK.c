/*
 * Reference: http://blog.oasisfeng.com/2006/10/19/full-cjk-unicode-range/
 *
 * Copyright (c) 2012-2014 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "../../src/bsdconv.h"

static const struct uint32_range ranges[] = {
	{ 0x3100, 0x312F },	//Chinese Bopomofo
	{ 0x3400, 0x4DB5 },	//CJK Unified Ideographs Extension A	;Unicode3.0
	{ 0x4E00, 0x6FFF },	//CJK Unified Ideographs	;Unicode 1.1	;HF
	{ 0x7000, 0x9FA5 },	//CJK Unified Ideographs	;Unicode 1.1	;LF
	{ 0x9FA6, 0x9FBB },	//CJK Unified Ideographs	;Unicode 4.1
	{ 0xF900, 0xFA2D },	//CJK Compatibility Ideographs	;Unicode 1.1
	{ 0xFA30, 0xFA6A },	//CJK Compatibility Ideographs	;Unicode 3.2
	{ 0xFA70, 0xFAD9 },	//CJK Compatibility Ideographs	;Unicode 4.1
	{ 0x20000, 0x2A6D6 },//CJK Unified Ideographs Extension B	;Unicode 3.1
	{ 0x2F800, 0x2FA1D },//CJK Compatibility Supplement	;Unicode 3.1
};

#include "unicode_range.c"