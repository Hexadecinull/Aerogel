#![no_std]
#![allow(clippy::missing_safety_doc)]

pub mod net;

use net::{arp, icmp, ipv4, ethernet::EtherType};
use core::panic::PanicInfo;

#[panic_handler]
fn panic(_: &PanicInfo) -> ! { loop {} }

/// Process a received Ethernet frame.
/// Returns true and fills `reply_buf`/`reply_len` if a reply should be sent.
///
/// # Safety
/// All pointers must be valid for their respective lengths.
#[no_mangle]
pub unsafe extern "C" fn net_process_packet(
    data:       *const u8,
    len:        u32,
    our_mac:    *const u8,
    our_ip:     u32,
    reply_buf:  *mut u8,
    reply_len:  *mut u32,
) -> bool {
    if data.is_null() || our_mac.is_null() || reply_buf.is_null() { return false; }

    let pkt     = core::slice::from_raw_parts(data, len as usize);
    let omac    = core::slice::from_raw_parts(our_mac, 6);
    let rbuf    = core::slice::from_raw_parts_mut(reply_buf, 1514);

    if pkt.len() < 14 { return false; }

    let etype   = EtherType::from_u16(u16::from_be_bytes([pkt[12], pkt[13]]));
    let src_mac = &pkt[6..12];
    let payload = &pkt[14..];

    let mut omac6 = [0u8; 6];
    omac6.copy_from_slice(omac);

    let mut smac6 = [0u8; 6];
    smac6.copy_from_slice(src_mac);

    let n = match etype {
        EtherType::Arp => arp::handle(payload, &omac6, our_ip, &smac6, rbuf),
        EtherType::Ipv4 => {
            if let Some((ip_payload, proto, src_ip, dst_ip)) = ipv4::parse(payload) {
                if dst_ip != our_ip { return false; }
                match proto {
                    ipv4::PROTO_ICMP => icmp::handle(
                        ip_payload, &omac6, our_ip, &smac6, src_ip, rbuf
                    ),
                    _ => 0,
                }
            } else { 0 }
        }
        _ => 0,
    };

    if n > 0 {
        *reply_len = n as u32;
        true
    } else {
        false
    }
}

/// Build a gratuitous ARP announcement.
///
/// # Safety
/// `our_mac` must point to 6 bytes; `out` must have at least 42 bytes.
#[no_mangle]
pub unsafe extern "C" fn net_arp_announce(
    our_mac: *const u8,
    our_ip:  u32,
    out:     *mut u8,
) -> u32 {
    if our_mac.is_null() || out.is_null() { return 0; }
    let omac = core::slice::from_raw_parts(our_mac, 6);
    let buf  = core::slice::from_raw_parts_mut(out, 42);

    let bcast: [u8; 6] = [0xFF; 6];
    let mut omac6 = [0u8; 6];
    omac6.copy_from_slice(omac);
    let smac6 = omac6;

    let n = arp::handle(
        &{
            let mut fake = [0u8; 28];
            fake[0..2].copy_from_slice(&[0x00, 0x01]);
            fake[2..4].copy_from_slice(&[0x08, 0x00]);
            fake[4] = 6; fake[5] = 4;
            fake[6..8].copy_from_slice(&[0x00, 0x01]);
            fake[8..14].copy_from_slice(&omac6);
            fake[14..18].copy_from_slice(&our_ip.to_be_bytes());
            fake[24..28].copy_from_slice(&our_ip.to_be_bytes());
            fake
        },
        &omac6, our_ip, &smac6, buf,
    );
    let _ = bcast;
    n as u32
}
