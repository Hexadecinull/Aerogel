use crate::net::{ethernet, ipv4};

pub fn handle(
    payload:  &[u8],
    our_mac:  &[u8;6],
    our_ip:   u32,
    src_mac:  &[u8;6],
    src_ip:   u32,
    out:      &mut [u8],
) -> usize {
    if payload.len() < 8 { return 0; }
    let icmp_type = payload[0];
    if icmp_type != 8 { return 0; }  // only handle echo request

    let data_len = payload.len();
    if out.len() < 14 + 20 + data_len { return 0; }

    let mut src_mac_arr = [0u8;6];
    src_mac_arr.copy_from_slice(src_mac);

    let mut eh = ethernet::write_frame(out, &src_mac_arr, our_mac, 0x0800);

    let ip_buf = &mut out[eh..];
    let ih = ipv4::write(ip_buf, ipv4::PROTO_ICMP, our_ip, src_ip, data_len);
    eh += ih;

    let icmp = &mut out[eh..eh + data_len];
    icmp.copy_from_slice(payload);
    icmp[0] = 0;  // type = echo reply
    icmp[1] = 0;  // code = 0
    icmp[2] = 0;  // zero checksum before computing
    icmp[3] = 0;
    let csum = ipv4::checksum(&icmp[..data_len]);
    icmp[2..4].copy_from_slice(&csum.to_be_bytes());

    eh + data_len
}
