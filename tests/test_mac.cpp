#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

extern "C" {
#include "mac.h"
}

namespace {

using Octets = std::array<uint8_t, MAC_ADDR_LEN>;
using TextBuf = std::array<char, MAC_ADDR_STR_SIZE>;

constexpr char kCanonical[] = "AA:BB:CC:DD:EE:FF";
constexpr Octets kCanonicalOctets{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

mac_addr_t make_mac(const Octets &octets) {
  mac_addr_t mac{};
  for (std::size_t i = 0; i < octets.size(); ++i) {
    mac.octets[i] = octets[i];
  }
  return mac;
}

Octets octets_of(const mac_addr_t &mac) {
  Octets result{};
  for (std::size_t i = 0; i < result.size(); ++i) {
    result[i] = mac.octets[i];
  }
  return result;
}

void expect_rejected(const char *text) {
  mac_addr_t mac = make_mac(Octets{0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02});
  const Octets before = octets_of(mac);
  EXPECT_EQ(mac_deserialize(text, &mac), -1) << "input: \"" << text << "\"";
  EXPECT_EQ(octets_of(mac), before)
      << "MAC must stay unchanged on error; input: \"" << text << "\"";
}

class MacTest : public ::testing::Test {
protected:
  TextBuf out_{};
};

TEST_F(MacTest, SerializesCanonicalUppercase) {
  const mac_addr_t mac = make_mac(kCanonicalOctets);
  ASSERT_EQ(mac_serialize(&mac, out_.data(), out_.size()), MAC_ADDR_STR_LEN);
  EXPECT_STREQ(out_.data(), kCanonical);
}

TEST_F(MacTest, PadsEachOctetToTwoUppercaseHexDigits) {
  const mac_addr_t mac = make_mac(Octets{0x00, 0x01, 0x0A, 0x7F, 0x10, 0xFF});
  ASSERT_EQ(mac_serialize(&mac, out_.data(), out_.size()), MAC_ADDR_STR_LEN);
  EXPECT_STREQ(out_.data(), "00:01:0A:7F:10:FF");
}

TEST_F(MacTest, DoesNotWritePastBufferEnd) {
  struct Guarded {
    char text[MAC_ADDR_STR_SIZE];
    char guard[8];
  };
  Guarded guarded{};
  std::memset(&guarded, '?', sizeof guarded);

  const mac_addr_t mac = make_mac(kCanonicalOctets);
  ASSERT_EQ(mac_serialize(&mac, guarded.text, sizeof guarded.text),
            MAC_ADDR_STR_LEN);
  EXPECT_STREQ(guarded.text, kCanonical);
  for (char c : guarded.guard) {
    EXPECT_EQ(c, '?');
  }
}

TEST_F(MacTest, RejectsBufferWithoutRoomForTerminator) {
  const mac_addr_t mac = make_mac(kCanonicalOctets);
  std::array<char, MAC_ADDR_STR_LEN> small;
  small.fill('?');

  EXPECT_EQ(mac_serialize(&mac, small.data(), small.size()), -1);
  for (char c : small) {
    EXPECT_EQ(c, '?');
  }
}

TEST_F(MacTest, RejectsEmptyBuffer) {
  const mac_addr_t mac = make_mac(kCanonicalOctets);
  out_.fill('?');

  EXPECT_EQ(mac_serialize(&mac, out_.data(), 0), -1);
  for (char c : out_) {
    EXPECT_EQ(c, '?');
  }
}

TEST_F(MacTest, RejectsNullArguments) {
  const mac_addr_t mac = make_mac(kCanonicalOctets);

  EXPECT_EQ(mac_serialize(nullptr, out_.data(), out_.size()), -1);
  EXPECT_EQ(mac_serialize(&mac, nullptr, 0), -1);
  EXPECT_EQ(mac_serialize(&mac, nullptr, MAC_ADDR_STR_SIZE), -1);
}

TEST_F(MacTest, ParsesCanonicalUppercase) {
  mac_addr_t mac{};
  ASSERT_EQ(mac_deserialize(kCanonical, &mac), 0);
  EXPECT_EQ(octets_of(mac), kCanonicalOctets);
}

TEST_F(MacTest, ParsesLowercase) {
  mac_addr_t mac{};
  ASSERT_EQ(mac_deserialize("aa:bb:cc:dd:ee:ff", &mac), 0);
  EXPECT_EQ(octets_of(mac), kCanonicalOctets);
}

TEST_F(MacTest, ParsesMixedCase) {
  mac_addr_t mac{};
  ASSERT_EQ(mac_deserialize("aA:Bb:cC:Dd:eE:fF", &mac), 0);
  EXPECT_EQ(octets_of(mac), kCanonicalOctets);
}

TEST_F(MacTest, RejectsWrongLength) {
  expect_rejected("");
  expect_rejected("AA:BB:CC:DD:EE:");
  expect_rejected("AA:BB:CC:DD:EE:F");
  expect_rejected("AA:BB:CC:DD:EE:FFF");
  expect_rejected(" AA:BB:CC:DD:EE:FF");
  expect_rejected("AA:BB:CC:DD:EE:FF ");
  expect_rejected("AA:BB:CC:DD:EE:FF\n");
}

TEST_F(MacTest, RejectsWrongSeparators) {
  expect_rejected("AA-BB-CC-DD-EE-FF");
  expect_rejected("AA:BB:CC:DD:EE/FF");
  expect_rejected("AA:BB:CC:DD:EE;FF");
  expect_rejected("AA.BB.CC.DD.EE.FF");
}

TEST_F(MacTest, RejectsNonHexCharacters) {
  expect_rejected("GG:BB:CC:DD:EE:FF");
  expect_rejected("AA:BB:CC:DD:EE:FG");
  expect_rejected("AA:BB:CC:DD:EE:F ");
}

TEST_F(MacTest, RejectsNullText) {
  mac_addr_t mac = make_mac(Octets{0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02});
  const Octets before = octets_of(mac);
  EXPECT_EQ(mac_deserialize(nullptr, &mac), -1);
  EXPECT_EQ(octets_of(mac), before);
}

TEST_F(MacTest, RejectsNullDestination) {
  EXPECT_EQ(mac_deserialize(kCanonical, nullptr), -1);
}

TEST_F(MacTest, RoundTripsAllOctetValues) {
  for (unsigned value = 0; value < 256; ++value) {
    const uint8_t v = static_cast<uint8_t>(value);
    const Octets octets{v,    static_cast<uint8_t>(255 - value),  0x00,
                        0xFF, static_cast<uint8_t>(value ^ 0x5A), v};
    const mac_addr_t mac = make_mac(octets);

    TextBuf text{};
    ASSERT_EQ(mac_serialize(&mac, text.data(), text.size()), MAC_ADDR_STR_LEN);

    mac_addr_t parsed{};
    ASSERT_EQ(mac_deserialize(text.data(), &parsed), 0) << text.data();
    EXPECT_EQ(octets_of(parsed), octets) << text.data();
  }
}

} // namespace
