/*
 * libopenraw - utils.rs
 *
 * Copyright (C) 2022 Hubert Figuière
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

/// Create an mut u8 slice from a mut u16.
pub(crate) fn to_u8_slice_mut(slice: &mut [u16]) -> &mut [u8] {
    unsafe { std::slice::from_raw_parts_mut(slice.as_mut_ptr().cast::<u8>(), 2 * slice.len()) }
}

/// Create an u8 slice from an u16.
pub(crate) fn to_u8_slice(slice: &[u16]) -> &[u8] {
    unsafe { std::slice::from_raw_parts(slice.as_ptr().cast::<u8>(), 2 * slice.len()) }
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
        // We'll try as a fallback.
        String::from_utf8_lossy(buf)
            .trim_end_matches(char::from(0))
            .to_string()
    }
}

#[cfg(test)]
mod test {
    use super::from_maybe_nul_terminated;

    #[test]
    fn test_from_maybe_nul_terminated() {
        let s1 = b"abcdef\0";
        let s2 = b"abcdef";
        let s3 = b"abcdef\0\0";

        assert_eq!(from_maybe_nul_terminated(s1), "abcdef");
        assert_eq!(from_maybe_nul_terminated(s2), "abcdef");
        assert_eq!(from_maybe_nul_terminated(s3), "abcdef");
    }
}
