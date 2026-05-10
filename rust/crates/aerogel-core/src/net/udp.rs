pub fn parse(pkt: &[u8]) -> Option<(u16, u16, &[u8])> {
    if pkt.len() < 8 { return None; }
    let src_port = u16::from_be_bytes([pkt[0], pkt[1]]);
    let dst_port = u16::from_be_bytes([pkt[2], pkt[3]]);
    Some((src_port, dst_port, &pkt[8..]))
}

pub fn write(buf: &mut [u8], src_port: u16, dst_port: u16, payload: &[u8]) -> usize {
    let total = 8 + payload.len();
    if buf.len() < total { return 0; }
    buf[0..2].copy_from_slice(&src_port.to_be_bytes());
    buf[2..4].copy_from_slice(&dst_port.to_be_bytes());
    buf[4..6].copy_from_slice(&(total as u16).to_be_bytes());
    buf[6..8].copy_from_slice(&[0,0]);  // checksum (optional for UDP)
    buf[8..8+payload.len()].copy_from_slice(payload);
    total
}
