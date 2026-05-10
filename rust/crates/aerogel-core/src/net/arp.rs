use crate::net::ethernet;

pub fn handle(
    payload:  &[u8],
    our_mac:  &[u8;6],
    our_ip:   u32,
    src_mac:  &[u8;6],
    out:      &mut [u8],
) -> usize {
    if payload.len() < 28 { return 0; }

    let oper     = u16::from_be_bytes([payload[6], payload[7]]);
    let sender_ip = u32::from_be_bytes([payload[14],payload[15],payload[16],payload[17]]);
    let target_ip = u32::from_be_bytes([payload[24],payload[25],payload[26],payload[27]]);

    if oper != 1 || target_ip != our_ip { return 0; }

    let mut sender_mac = [0u8;6];
    sender_mac.copy_from_slice(&payload[8..14]);

    let hlen = ethernet::write_frame(out, &sender_mac, our_mac, 0x0806);
    if out.len() < hlen + 28 { return 0; }
    let p = &mut out[hlen..];

    p[0..2].copy_from_slice(&[0x00,0x01]);  // HTYPE = Ethernet
    p[2..4].copy_from_slice(&[0x08,0x00]);  // PTYPE = IPv4
    p[4] = 6; p[5] = 4;                     // HLEN/PLEN
    p[6..8].copy_from_slice(&[0x00,0x02]);  // OPER = reply
    p[8..14].copy_from_slice(our_mac);       // SHA
    p[14..18].copy_from_slice(&our_ip.to_be_bytes());  // SPA
    p[18..24].copy_from_slice(&sender_mac);  // THA
    p[24..28].copy_from_slice(&sender_ip.to_be_bytes()); // TPA

    let _ = src_mac;
    hlen + 28
}
