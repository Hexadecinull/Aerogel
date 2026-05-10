#[repr(u16)]
#[derive(Copy, Clone, PartialEq)]
pub enum EtherType {
    Arp  = 0x0806,
    Ipv4 = 0x0800,
    Unknown,
}

impl EtherType {
    pub fn from_u16(v: u16) -> Self {
        match v {
            0x0806 => EtherType::Arp,
            0x0800 => EtherType::Ipv4,
            _      => EtherType::Unknown,
        }
    }
}

pub fn write_frame(buf: &mut [u8], dst: &[u8;6], src: &[u8;6], etype: u16) -> usize {
    if buf.len() < 14 { return 0; }
    buf[0..6].copy_from_slice(dst);
    buf[6..12].copy_from_slice(src);
    buf[12] = (etype >> 8) as u8;
    buf[13] = etype as u8;
    14
}
