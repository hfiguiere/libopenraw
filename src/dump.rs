//! Dumping the file content.

/// Trait to dump a container.
pub trait Dump {
    /// Print dump the container as `indent`
    fn print_dump(&self, indent: u32);
}

pub fn dump_indent(indent: u32) -> String {
    let mut s = String::with_capacity(indent as usize);
    for _ in 0..indent {
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
