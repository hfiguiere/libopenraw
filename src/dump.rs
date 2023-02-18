// SPDX-License-Identifier: LGPL-3.0-or-later

//! Dumping the file content.

#[cfg(feature = "dump")]
use std::collections::HashMap;

pub trait DumpFile {
    #[cfg(feature = "dump")]
    fn dump_file(&self, out: &mut dyn std::io::Write);
}

#[macro_export]
macro_rules! dumpfile_impl {
    ( $t:ty ) => {
        impl $crate::dump::DumpFile for $t {
            #[cfg(feature = "dump")]
            fn dump_file(&self, out: &mut dyn std::io::Write) {
                self.write_dump(out, 0);
            }
        }
    };
}

/// Trait to dump a container.
/// XXX invert the default methods
pub trait Dump {
    #[cfg(feature = "dump")]
    /// Print dump the container as `indent`
    fn write_dump<W>(&self, out: &mut W, indent: u32)
    where
        W: std::io::Write + ?Sized;

    #[cfg(feature = "dump")]
    /// Pass args to the print_dump
    fn write_dump_with_args<W>(&self, out: &mut W, indent: u32, _args: HashMap<&str, String>)
    where
        W: std::io::Write + ?Sized,
    {
        self.write_dump(out, indent);
    }
}

#[cfg(feature = "dump")]
pub fn dump_indent(indent: u32) -> String {
    let mut s = String::with_capacity(indent as usize);
    for _ in 0..indent {
        s.push(' ');
        s.push(' ');
    }

    s
}

#[macro_export]
macro_rules! dump_writeln {
    ( $out:expr, $indent:expr, $( $x:expr ),* ) => {
        {
            $out.write_all(&$crate::dump::dump_indent( $indent ).into_bytes()).unwrap();
            writeln!($out, $( $x ),*).unwrap();
        }
    };
}
