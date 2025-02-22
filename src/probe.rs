// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - auditor.rs
 *
 * Copyright (C) 2024-2025 Hubert Figuière
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

use std::cell::RefCell;
use std::collections::BTreeMap;

#[macro_export]
macro_rules! probe {
    ( $audit:expr, $key:expr, $value:expr ) => {{
        if let Some(probe) = &$audit {
            probe.set($key, $value)
        }
    }};
}

#[macro_export]
macro_rules! probe_imp {
    ( ) => {
        fn set_probe(&mut self, probe: bool) {
            self.probe = if probe {
                Some($crate::Probe::default())
            } else {
                None
            };
        }

        fn probe(&self) -> Option<&$crate::Probe> {
            self.probe.as_ref()
        }
    };
}

/// A class to gather features and quirks from the parsing.
#[derive(Debug, Default)]
pub struct Probe {
    audit: RefCell<BTreeMap<String, String>>,
}

impl Probe {
    pub(crate) fn set<T>(&self, key: &str, value: T)
    where
        T: ToString,
    {
        self.audit
            .borrow_mut()
            .insert(key.into(), value.to_string());
    }

    pub fn print_str(&self) -> String {
        let lines = self
            .audit
            .borrow()
            .iter()
            .map(|(key, value)| format!("{key}: {value}"))
            .collect::<Vec<String>>();
        lines.join("\n")
    }
}
