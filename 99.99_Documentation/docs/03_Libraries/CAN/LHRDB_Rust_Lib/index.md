# LHRDB Rust Library

The LHRDB Rust Library is a set of structs and enums that are generated
automatically by the Rust `dbc-codegen` crate. These make it much easier to
write programs that output or consume signals that are produced via CAN.

## Adding to a Rust Project

Adding the LHRDB CAN Library can be added to a rust project for any chip in the
same way. Simply add the following to your dependencies in the `cargo.toml`:

```toml hl_lines="2"
[dependencies]
lhrdb = { path = "../../10-19_Libraries/12_CAN/12.02_DBC_Rust_Library" }
```

Make sure that you put the correct path, relative to where your project is
located. 

## Using the Library

Using the library is quite easy. Here is a sample of how you can encode and
decode messages:

```rust
use embedded_can::{Frame, Id, StandardId};
use lhrdb::messages;

fn main() {
    let msg = messages::Id363::new(100, 100., 100.).unwrap();
    let id = StandardId::new(0x363).unwrap();
    let frame = Frame::new(id, msg.data());

    can_hardware.transmit(&frame).unwrap();

    // ... something to get a message

    messages::Messages::from_can_message(can_id, bytes);
}
```

You can also view all of the library functions by calling `cargo doc --open`
when you are in the `12.02` folder.
