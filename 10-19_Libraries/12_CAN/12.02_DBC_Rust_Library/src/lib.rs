mod messages {
    include!(concat!(env!("OUT_DIR"), "/messages.rs"));
}

fn test() {
    let message = messages::Id400::new(8).unwrap();
}
