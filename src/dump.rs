//! Dumping the file content.

use std::collections::HashMap;

/// Trait to dump a container.
/// XXX invert the default methods
pub trait Dump {
    /// Print dump the container as `indent`
    fn print_dump(&self, indent: u32);

    /// Pass args to the print_dump
    fn print_dump_with_args(&self, indent: u32, _args: HashMap<&str, String>) {
        self.print_dump(indent);
    }
}

pub fn dump_indent(indent: u32) -> String {
    let mut s = String::with_capacity(indent as usize);
    for _ in 0..indent {
        s.push(' ');
        s.push(' ');
    }

    s
}

#[macro_export]
macro_rules! dump_println {
    ( $indent:expr, $( $x:expr ),* ) => {
        {
            use crate::dump::dump_indent;
            print!("{}", dump_indent( $indent ));
            println!($( $x ),*);
        }
    };
}
