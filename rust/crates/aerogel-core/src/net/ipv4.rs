pub const PROTO_ICMP: u8 = 1;
pub const PROTO_UDP:  u8 = 17;
pub const PROTO_TCP:  u8 = 6;

pub fn checksum(data: &[u8]) -> u16 {
    let mut sum: u32 = 0;
    let mut i = 0;
    while i + 1 < data.len() {
        sum += u16::from_be_bytes([data[i], data[i+1]]) as u32;
        i += 2;
    }
    if i < data.len() { sum += (data[i] as u32) << 8; }
    while sum >> 16 != 0 { sum = (sum & 0xFFFF) + (sum >> 16); }
    !(sum as u16)
}

pub fn parse<'a>(pkt: &'a [u8]) -> Option<(&'a [u8], u8, u32, u32)> {
    if pkt.len() < 20 { return None; }
    let ihl    = (pkt[0] & 0x0F) as usize * 4;
    let proto  = pkt[9];
    let src_ip = u32::from_be_bytes([pkt[12],pkt[13],pkt[14],pkt[15]]);
    let dst_ip = u32::from_be_bytes([pkt[16],pkt[17],pkt[18],pkt[19]]);
    if pkt.len() < ihl { return None; }
    Some((&pkt[ihl..], proto, src_ip, dst_ip))
}

pub fn write(
    buf: &mut [u8], proto: u8,
    src_ip: u32, dst_ip: u32, payload_len: usize,
) -> usize {
    let total = 20 + payload_len;
    if buf.len() < total { return 0; }
    buf[0]  = 0x45;
    buf[1]  = 0;
    buf[2..4].copy_from_slice(&(total as u16).to_be_bytes());
    buf[4..6].copy_from_slice(&[0,1]);
    buf[6..8].copy_from_slice(&[0x40,0]);  // DF flag
    buf[8]  = 64;
    buf[9]  = proto;
    buf[10..12].copy_from_slice(&[0,0]);
    buf[12..16].copy_from_slice(&src_ip.to_be_bytes());
    buf[16..20].copy_from_slice(&dst_ip.to_be_bytes());
    let csum = checksum(&buf[..20]);
    buf[10..12].copy_from_slice(&csum.to_be_bytes());
    20
}
