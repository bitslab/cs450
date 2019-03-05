use std::net::UdpSocket;
use std::net::{Ipv4Addr, SocketAddrV4};

use std::fs::File;
use std::env;
use std::io::prelude::*;


fn main() {
    wrapper();
}

fn wrapper() -> std::io::Result<()> {
    let addr : Ipv4Addr=env::args().nth(1).unwrap().parse().unwrap();
    let port : u16=env::args().nth(2).unwrap().parse().unwrap();
    
    let mut socket = UdpSocket::bind("0.0.0.0:8765")?;

    let mut file = File::open(env::args().nth(3).unwrap())?;
    let mut buf=[0u8; 100];    
    file.read(&mut buf)?;

    socket.send_to(&buf,SocketAddrV4::new(addr,port));
    Ok(())
}

