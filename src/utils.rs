// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - utils.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

//! Various utilities

#[macro_export]
/// Create a vecto with size but not initialized.
/// Use with caution.
macro_rules! uninit_vec {
    ( $len:expr ) => {{
        let mut v = Vec::with_capacity($len);
        #[allow(clippy::uninit_vec)]
        unsafe {
            v.set_len($len)
        };
        v
    }};
}

/// Convert a slice of u32 to a quad of u16
/// If there is less than 4 in the slice, it will use the first
/// value for the quad.
pub(crate) fn to_quad(slice: &[u32]) -> [u16; 4] {
    if slice.len() != 4 {
        [slice[0] as u16; 4]
    } else {
        let mut sl = [0_u16; 4];
        for (idx, v) in slice[0..4].iter().enumerate() {
            sl[idx] = *v as u16;
        }
        sl
    }
}

/// Create an mut u8 slice from a `mut [T]`.
pub(crate) fn to_u8_slice_mut<T>(slice: &mut [T]) -> &mut [u8] {
    unsafe {
        std::slice::from_raw_parts_mut(
            slice.as_mut_ptr().cast::<u8>(),
            std::mem::size_of_val(slice),
        )
    }
}

/// Create an u8 slice from a `[T]`.
pub(crate) fn to_u8_slice<T>(slice: &[T]) -> &[u8] {
    unsafe { std::slice::from_raw_parts(slice.as_ptr().cast::<u8>(), std::mem::size_of_val(slice)) }
}

/// Will take the slice and create a string from the nul terminated
/// content.
/// Will fallback (slower path) if
/// * there is no nul terminator.
/// * there are many nuls after the string.
/// This is used for Exif and RAF file where the strings are nul
/// terminated.
/// We use lossy from utf8.
pub(crate) fn from_maybe_nul_terminated(buf: &[u8]) -> String {
    if let Ok(cstr) = std::ffi::CStr::from_bytes_with_nul(buf) {
        cstr.to_string_lossy().to_string()
    } else {
        // Split at the first NUL in case there is more.
        let mut s = buf.split(|v| *v == 0);
        if let Some(buf) = s.next() {
            String::from_utf8_lossy(buf).to_string()
        } else {
            // We'll try as a fallback.
            String::from_utf8_lossy(buf)
                .trim_end_matches(char::from(0))
                .to_string()
        }
    }
}

/// Convert the bytes of a string to a `Vec<u8>` with a NUL
/// This is useful for C API.
pub(crate) fn to_nul_terminated(buf: &str) -> Vec<u8> {
    let mut s = Vec::with_capacity(buf.len() + 1);
    s.extend_from_slice(buf.as_bytes());
    s.push(0);
    s
}

#[cfg(test)]
mod test {
    use super::from_maybe_nul_terminated;

    #[test]
    fn test_from_maybe_nul_terminated() {
        let s1 = b"abcdef\0";
        let s2 = b"abcdef";
        let s3 = b"abcdef\0\0";
        let s4 = vec![0_u8, 255, 255, 255];
        let s5 = b"abcdef\0\xff\xff\xff";

        assert_eq!(from_maybe_nul_terminated(s1), "abcdef");
        assert_eq!(from_maybe_nul_terminated(s2), "abcdef");
        assert_eq!(from_maybe_nul_terminated(s3), "abcdef");
        assert_eq!(from_maybe_nul_terminated(&s4), "");
        assert_eq!(from_maybe_nul_terminated(s5), "abcdef");
    }
}
