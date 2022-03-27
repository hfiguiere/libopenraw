/*
 * libopenraw - decompress/sliced_buffer.rs
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

//! Sliced buffer, mostly for Canon. No relation with Rust slices.
//!
//! Layout:
//! H and W = Height and Width of picture
//! S and s' = slice width. Canon has on s' slice and
//!            as many S needed with sum(S,s') = W
//!```text
//!
//!                W
//!           S     S    s'
//!        +-----+-----+---+
//!        | 1   | 5   | 9 |
//!        | 2   | 6   | 10|
//!     H  | 3   | 7   | 11|
//!        | 4   | 8   | 12|
//!        +-----+-----+---+
//!```
//!

/// Sliced buffer, ie buffer with Canon slices
/// For raw.
pub(crate) struct SlicedBuffer<T: Copy> {
    width: u16,
    height: u16,
    buffer: Vec<T>,
    /// Width of each slice.
    slices: Vec<usize>,
    /// Current slice
    slice: usize,
    slice_offset: usize,
    slice_width: usize,
    row_offset: usize,
    /// Current position to write in the buffer
    pos: usize,
}

impl<T: Copy> From<SlicedBuffer<T>> for Vec<T> {
    fn from(sb: SlicedBuffer<T>) -> Vec<T> {
        sb.buffer
    }
}

impl<T: Copy + Default> SlicedBuffer<T> {
    pub fn new(width: u16, height: u16, slices: Option<&[u16]>) -> SlicedBuffer<T> {
        let slices = slices.map(Vec::from).unwrap_or_else(|| vec![width]);

        let mut buffer: Vec<T> = Vec::new();
        buffer.resize(width as usize * height as usize, T::default());
        SlicedBuffer {
            width,
            height,
            buffer,
            slices: slices.iter().map(|v| usize::from(*v)).collect(),
            slice: 0,
            slice_offset: 0,
            slice_width: slices[0] as usize,
            row_offset: 0,
            pos: 0,
        }
    }

    pub fn reserve(&mut self, reserve_size: usize) {
        self.buffer.reserve(reserve_size)
    }

    /// Move to next slice
    fn next_slice(&mut self) {
        if self.slices.len() > self.slice {
            self.slice_offset += self.slices[self.slice];
            self.slice += 1;
        }
        self.slice_width = if self.slices.len() > self.slice {
            self.slices[self.slice]
        } else {
            0
        };
    }

    /// Move to the next row
    fn next_row(&mut self) -> usize {
        let w = self.width;
        let mut row: usize = (self.pos / w as usize) + 1;
        if row == self.height as usize {
            self.next_slice();
            row = 0
        }
        let new_pos = row * w as usize + self.slice_offset;
        self.row_offset = new_pos;

        new_pos
    }

    /// Calculate the next position
    fn advance(&mut self) -> usize {
        if self.pos + 1 - self.row_offset >= self.slice_width {
            self.next_row()
        } else {
            self.pos + 1
        }
    }

    /// Append data to the buffer.
    pub fn append(&mut self, data: &[T]) {
        if self.slices.len() == 1 {
            let new_pos = self.pos + data.len();
            self.buffer.as_mut_slice()[self.pos..new_pos].copy_from_slice(data);
            self.pos = new_pos;
        } else {
            for d in data {
                self.buffer[self.pos] = *d;
                self.pos = self.advance();
            }
        }
    }
}

#[cfg(test)]
mod test {

    use super::SlicedBuffer;

    #[test]
    fn test_noslice_sliced_buffer() {
        let bytes = b"abcdefghijklmnopqrstuvwxyz";
        let mut buffer: SlicedBuffer<u8> = SlicedBuffer::new(160, 120, None);

        assert_eq!(buffer.slices.len(), 1);
        assert_eq!(buffer.slices[0], 160);
        buffer.append(bytes);

        assert_eq!(buffer.pos, bytes.len());
    }

    #[test]
    fn test_advance_sliced_buffer() {
        let w = 298_u16;
        let h = 100_u16;

        let slices = vec![128_u16, 128_u16, 42_u16];
        let mut buffer: SlicedBuffer<u16> = SlicedBuffer::new(w, h, Some(&slices));

        assert_eq!(buffer.pos, 0);
        assert_eq!(buffer.slice, 0);
        assert_eq!(buffer.slice_width, 128);
        assert_eq!(buffer.slice_offset, 0);
        assert_eq!(buffer.row_offset, 0);

        for i in 0..h as usize {
            assert_eq!(buffer.pos, i * w as usize);
            assert_eq!(buffer.pos / w as usize, i);
            for _ in 0..128 {
                buffer.pos = buffer.advance();
            }
        }

        assert_eq!(buffer.slice_width, 128);
        assert_eq!(buffer.slice_offset, 128);
        assert_eq!(buffer.row_offset, 128);
        assert_eq!(buffer.pos, 128);
        assert_eq!(buffer.slice, 1);
    }

    #[test]
    fn test_sliced_buffer() {
        let w = 298_u16;
        let h = 100_u16;

        let mut source = vec![1_u16; 12800];
        source.extend_from_slice(&[2_u16; 12800]);
        source.extend_from_slice(&[3_u16; 4200]);
        assert_eq!(source.len(), w as usize * h as usize);

        let slices = vec![128_u16, 128_u16, 42_u16];
        assert_eq!(w, slices.iter().copied().sum::<u16>());

        let mut buffer: SlicedBuffer<u16> = SlicedBuffer::new(w, h, Some(&slices));
        for v in source {
            buffer.append(&[v]);
        }
        assert_eq!(buffer.buffer.len(), 29800);
        assert_eq!(buffer.buffer[0], 1);
        assert_eq!(buffer.buffer[128], 2);
        assert_eq!(buffer.buffer[256], 3);

        assert_eq!(buffer.buffer[298], 1);
    }
}
