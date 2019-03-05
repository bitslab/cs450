use std::net::UdpSocket;
use std::net::{Ipv4Addr, SocketAddrV4};
use std::io::{self, Write};

use std::fs::File;
use std::env;
use std::io::prelude::*;


fn main() {
    wrapper();
}

fn wrapper() -> std::io::Result<()> {
    let port : u16=env::args().nth(1).unwrap().parse().unwrap();
    
    let mut socket = UdpSocket::bind(format!("0.0.0.0:{}",port))?;

    let mut buf=[0u8; 100];    
    let (bytes, from) = socket.recv_from(&mut buf)?;
    io::stdout().write(&buf);
    
    Ok(())
}

