// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - mosaic.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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

//! CFA Mosaic

use num_enum::TryFromPrimitive;

/// C friendly enum for Pattern types.
#[repr(u8)]
pub enum PatternType {
    None = 0,
    NonRgb22 = 1,
    Rggb = 2,
    Gbrg = 3,
    Bggr = 4,
    Grbg = 5,
}

impl From<&Pattern> for PatternType {
    fn from(value: &Pattern) -> PatternType {
        match *value {
            Pattern::Empty => PatternType::None,
            Pattern::NonRgb22(_) => PatternType::NonRgb22,
            Pattern::Rggb => PatternType::Rggb,
            Pattern::Gbrg => PatternType::Gbrg,
            Pattern::Bggr => PatternType::Bggr,
            Pattern::Grbg => PatternType::Grbg,
        }
    }
}

/// A pattern colour component.
#[derive(Clone, Copy, Debug, PartialEq, TryFromPrimitive)]
#[repr(u8)]
pub enum PatternColour {
    Red = 0,
    Green = 1,
    Blue = 2,
}

impl PatternColour {
    pub fn to_char(self) -> char {
        match self {
            Self::Red => 'R',
            Self::Green => 'G',
            Self::Blue => 'B',
        }
    }
}

/// Describe a pattern
#[derive(Clone, Debug, Default, PartialEq)]
pub enum Pattern {
    /// Empty pattern. Most case it's an error.
    #[default]
    Empty,
    /// Non RGB22. Like X-Trans, the data is the actual pattern.
    NonRgb22(Vec<PatternColour>),
    /// RGGB 2x2 bayer
    Rggb,
    /// GBRG 2x2 bayer
    Gbrg,
    /// BGGR 2x2 bayer
    Bggr,
    /// GRBG 2x2 bayer
    Grbg,
}

impl ToString for Pattern {
    /// The `ToString` conversion for the pattern will print a string of
    /// the pattern colour filters left - right & top - bottom
    fn to_string(&self) -> String {
        match *self {
            Self::Empty => "NONE".into(),
            Self::Rggb => "RGGB".into(),
            Self::Gbrg => "GBRG".into(),
            Self::Bggr => "BGGR".into(),
            Self::Grbg => "GRBG".into(),
            Self::NonRgb22(_) => "NON_RGB22".into(),
            //p.iter().map(|c| c.to_char()).collect(),
        }
    }
}

impl std::convert::TryFrom<&[u8]> for Pattern {
    type Error = &'static str;

    fn try_from(v: &[u8]) -> Result<Pattern, Self::Error> {
        use PatternColour::*;
        if v.len() == 4 {
            if v == [Red as u8, Green as u8, Green as u8, Blue as u8] {
                Ok(Self::Rggb)
            } else if v == [Green as u8, Blue as u8, Red as u8, Green as u8] {
                Ok(Self::Gbrg)
            } else if v == [Green as u8, Red as u8, Blue as u8, Green as u8] {
                Ok(Self::Grbg)
            } else if v == [Blue as u8, Green as u8, Green as u8, Red as u8] {
                Ok(Self::Bggr)
            } else {
                Err("Invalid pattern")
            }
        } else {
            let colours: Vec<PatternColour> = v
                .iter()
                .map(|colour| PatternColour::try_from_primitive(*colour))
                .take_while(Result::is_ok)
                .map(|v| v.unwrap())
                .collect();
            if colours.len() == v.len() {
                Ok(Self::NonRgb22(colours))
            } else {
                Err("Invalid colour found")
            }
        }
    }
}

impl Pattern {
    /// Return a `PatternType`, mostly for C API.
    pub fn pattern_type(&self) -> PatternType {
        self.into()
    }

    /// Return the pattern colour array
    pub fn pattern(&self) -> Vec<PatternColour> {
        use PatternColour::*;
        match *self {
            Self::Empty => Vec::default(),
            Self::Rggb => vec![Red, Green, Green, Blue],
            Self::Gbrg => vec![Green, Blue, Red, Green],
            Self::Bggr => vec![Blue, Green, Green, Red],
            Self::Grbg => vec![Green, Red, Blue, Green],
            Self::NonRgb22(ref p) => p.clone(),
        }
    }
}

#[cfg(test)]
mod test {
    use std::convert::TryFrom;

    use super::Pattern;
    use super::PatternColour::*;

    #[test]
    fn test_pattern_to_string() {
        let pattern = Pattern::default();
        assert_eq!(&pattern.to_string(), "NONE");

        let pattern = Pattern::Rggb;
        assert_eq!(&pattern.to_string(), "RGGB");

        let pattern = Pattern::NonRgb22(vec![Red, Green, Blue, Blue, Red, Green, Green, Blue, Red]);
        assert_eq!(&pattern.to_string(), "NON_RGB22");
    }

    #[test]
    fn test_pattern_try_from() {
        // Valid 2x2 pattern
        let pattern = vec![Red as u8, Green as u8, Green as u8, Blue as u8];
        assert_eq!(Pattern::try_from(pattern.as_slice()), Ok(Pattern::Rggb));

        // Invalid 2x2 pattern
        let pattern = vec![Green as u8, Green as u8, Red as u8, Blue as u8];
        assert!(Pattern::try_from(pattern.as_slice()).is_err());

        // Valid non 2x2 pattern
        let pattern = vec![
            Red as u8,
            Green as u8,
            Green as u8,
            Blue as u8,
            Red as u8,
            Green as u8,
            Green as u8,
            Blue as u8,
        ];
        assert_eq!(
            Pattern::try_from(pattern.as_slice()),
            Ok(Pattern::NonRgb22(vec![
                Red, Green, Green, Blue, Red, Green, Green, Blue
            ]))
        );

        // Test we return an error if one of the component is invalid
        let pattern = vec![
            Red as u8,
            Green as u8,
            100,
            Blue as u8,
            Red as u8,
            Green as u8,
            Green as u8,
            Blue as u8,
        ];
        assert!(Pattern::try_from(pattern.as_slice()).is_err());
    }
}
