//! Various utilities

/// Create an mut u8 slice from a mut u16.
pub(crate) fn to_u8_slice_mut(slice: &mut [u16]) -> &mut [u8] {
    unsafe { std::slice::from_raw_parts_mut(slice.as_mut_ptr().cast::<u8>(), 2 * slice.len()) }
}

/// Create an u8 slice from an u16.
pub(crate) fn to_u8_slice(slice: &[u16]) -> &[u8] {
    unsafe { std::slice::from_raw_parts(slice.as_ptr().cast::<u8>(), 2 * slice.len()) }
}
