/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

struct WhirlpoolProcessor
{
    WhirlpoolProcessor() noexcept   : bufferBits (0), bufferPos (0)
    {
        zeromem (bitLength, sizeof (bitLength));
        zeromem (buffer, sizeof (buffer));
        zeromem (hash, sizeof (hash));
    }

    void processStream (InputStream& input, int64 numBytesToRead, uint8* const result)
    {
        if (numBytesToRead < 0)
            numBytesToRead = std::numeric_limits<int64>::max();

        for (;;)
        {
            uint8 data[64];
            const int bytesRead = input.read (data, (int) jmin (numBytesToRead, (int64) sizeof (data)));

            add (data, bytesRead * 8);

            if (bytesRead < (int) sizeof (data))
                break;

            numBytesToRead -= sizeof (data);
        }

        finalize (result);
    }

    uint8 bitLength[32];
    uint8 buffer[64];
    int bufferBits, bufferPos;
    uint64 hash[8];

private:
    void add (const uint8* const source, int numBits) noexcept
    {
        int sourcePos = 0;                        // index of leftmost source uint8 containing data (1 to 8 bits)
        int sourceGap = (8 - (numBits & 7)) & 7;  // space on source[sourcePos]
        int bufferRem = bufferBits & 7;           // occupied bits on buffer[bufferPos]

        uint64 value = (uint64) numBits, carry = 0;

        for (int i = 32; --i >= 0 && (carry != 0 || value != 0);)
        {
            carry += bitLength[i] + ((uint32) value & 0xff);
            bitLength[i] = (uint8) carry;
            carry >>= 8;
            value >>= 8;
        }

        uint32 b = 0;

        while (numBits > 8)
        {
            b = ((source[sourcePos] << sourceGap) & 0xff)
                  | ((source[sourcePos + 1] & 0xff) >> (8 - sourceGap));

            buffer[bufferPos++] |= (uint8) (b >> bufferRem);
            bufferBits += 8 - bufferRem;

            if (bufferBits == 64 * 8)
            {
                processNextBuffer();
                bufferBits = bufferPos = 0;
            }

            buffer[bufferPos] = (uint8) (b << (8 - bufferRem));
            bufferBits += bufferRem;

            numBits -= 8;
            ++sourcePos;
        }

        if (numBits > 0)
        {
            b = (source[sourcePos] << sourceGap) & 0xff;
            buffer[bufferPos] |= (b >> bufferRem);
        }
        else
        {
            b = 0;
        }

        if (bufferRem + numBits < 8)
        {
            bufferBits += numBits;
        }
        else
        {
            ++bufferPos;
            bufferBits += 8 - bufferRem;
            numBits    -= 8 - bufferRem;

            if (bufferBits == 64 * 8)
            {
                processNextBuffer();
                bufferBits = bufferPos = 0;
            }

            buffer[bufferPos] = (uint8) (b << (8 - bufferRem));
            bufferBits += numBits;
        }
    }

    void finalize (uint8* const result) noexcept
    {
        // append a '1'-bit
        buffer[bufferPos++] |= 0x80u >> (bufferBits & 7);

        // pad with zero bits to complete (N*(64*8) - (32*8)) bits
        if (bufferPos > 32)
        {
            if (bufferPos < 64)
                zeromem (buffer + bufferPos, (size_t) (64 - bufferPos));

            processNextBuffer();
            bufferPos = 0;
        }

        if (bufferPos < 32)
            zeromem (buffer + bufferPos, (size_t) (32 - bufferPos));

        bufferPos = 32;
        memcpy (buffer + 32, bitLength, 32);  // append bit length of hashed data

        processNextBuffer();

        uint8* digest = result;

        for (int i = 0; i < 8; ++i)
        {
            digest[0] = (uint8) (hash[i] >> 56);
            digest[1] = (uint8) (hash[i] >> 48);
            digest[2] = (uint8) (hash[i] >> 40);
            digest[3] = (uint8) (hash[i] >> 32);
            digest[4] = (uint8) (hash[i] >> 24);
            digest[5] = (uint8) (hash[i] >> 16);
            digest[6] = (uint8) (hash[i] >>  8);
            digest[7] = (uint8)  hash[i];
            digest += 8;
        }
    }

    void processNextBuffer() noexcept
    {
        #undef X
        #define X(value) value##ULL

        static const uint64 rc[] =
        {   X(0x1823c6e887b8014f), X(0x36a6d2f5796f9152), X(0x60bc9b8ea30c7b35), X(0x1de0d7c22e4bfe57), X(0x157737e59ff04ada),
            X(0x58c9290ab1a06b85), X(0xbd5d10f4cb3e0567), X(0xe427418ba77d95d8), X(0xfbee7c66dd17479e), X(0xca2dbf07ad5a8333)
        };

        static const uint64 C0[] =
        {   X(0x18186018c07830d8), X(0x23238c2305af4626), X(0xc6c63fc67ef991b8), X(0xe8e887e8136fcdfb), X(0x878726874ca113cb), X(0xb8b8dab8a9626d11), X(0x0101040108050209), X(0x4f4f214f426e9e0d),
            X(0x3636d836adee6c9b), X(0xa6a6a2a6590451ff), X(0xd2d26fd2debdb90c), X(0xf5f5f3f5fb06f70e), X(0x7979f979ef80f296), X(0x6f6fa16f5fcede30), X(0x91917e91fcef3f6d), X(0x52525552aa07a4f8),
            X(0x60609d6027fdc047), X(0xbcbccabc89766535), X(0x9b9b569baccd2b37), X(0x8e8e028e048c018a), X(0xa3a3b6a371155bd2), X(0x0c0c300c603c186c), X(0x7b7bf17bff8af684), X(0x3535d435b5e16a80),
            X(0x1d1d741de8693af5), X(0xe0e0a7e05347ddb3), X(0xd7d77bd7f6acb321), X(0xc2c22fc25eed999c), X(0x2e2eb82e6d965c43), X(0x4b4b314b627a9629), X(0xfefedffea321e15d), X(0x575741578216aed5),
            X(0x15155415a8412abd), X(0x7777c1779fb6eee8), X(0x3737dc37a5eb6e92), X(0xe5e5b3e57b56d79e), X(0x9f9f469f8cd92313), X(0xf0f0e7f0d317fd23), X(0x4a4a354a6a7f9420), X(0xdada4fda9e95a944),
            X(0x58587d58fa25b0a2), X(0xc9c903c906ca8fcf), X(0x2929a429558d527c), X(0x0a0a280a5022145a), X(0xb1b1feb1e14f7f50), X(0xa0a0baa0691a5dc9), X(0x6b6bb16b7fdad614), X(0x85852e855cab17d9),
            X(0xbdbdcebd8173673c), X(0x5d5d695dd234ba8f), X(0x1010401080502090), X(0xf4f4f7f4f303f507), X(0xcbcb0bcb16c08bdd), X(0x3e3ef83eedc67cd3), X(0x0505140528110a2d), X(0x676781671fe6ce78),
            X(0xe4e4b7e47353d597), X(0x27279c2725bb4e02), X(0x4141194132588273), X(0x8b8b168b2c9d0ba7), X(0xa7a7a6a7510153f6), X(0x7d7de97dcf94fab2), X(0x95956e95dcfb3749), X(0xd8d847d88e9fad56),
            X(0xfbfbcbfb8b30eb70), X(0xeeee9fee2371c1cd), X(0x7c7ced7cc791f8bb), X(0x6666856617e3cc71), X(0xdddd53dda68ea77b), X(0x17175c17b84b2eaf), X(0x4747014702468e45), X(0x9e9e429e84dc211a),
            X(0xcaca0fca1ec589d4), X(0x2d2db42d75995a58), X(0xbfbfc6bf9179632e), X(0x07071c07381b0e3f), X(0xadad8ead012347ac), X(0x5a5a755aea2fb4b0), X(0x838336836cb51bef), X(0x3333cc3385ff66b6),
            X(0x636391633ff2c65c), X(0x02020802100a0412), X(0xaaaa92aa39384993), X(0x7171d971afa8e2de), X(0xc8c807c80ecf8dc6), X(0x19196419c87d32d1), X(0x494939497270923b), X(0xd9d943d9869aaf5f),
            X(0xf2f2eff2c31df931), X(0xe3e3abe34b48dba8), X(0x5b5b715be22ab6b9), X(0x88881a8834920dbc), X(0x9a9a529aa4c8293e), X(0x262698262dbe4c0b), X(0x3232c8328dfa64bf), X(0xb0b0fab0e94a7d59),
            X(0xe9e983e91b6acff2), X(0x0f0f3c0f78331e77), X(0xd5d573d5e6a6b733), X(0x80803a8074ba1df4), X(0xbebec2be997c6127), X(0xcdcd13cd26de87eb), X(0x3434d034bde46889), X(0x48483d487a759032),
            X(0xffffdbffab24e354), X(0x7a7af57af78ff48d), X(0x90907a90f4ea3d64), X(0x5f5f615fc23ebe9d), X(0x202080201da0403d), X(0x6868bd6867d5d00f), X(0x1a1a681ad07234ca), X(0xaeae82ae192c41b7),
            X(0xb4b4eab4c95e757d), X(0x54544d549a19a8ce), X(0x93937693ece53b7f), X(0x222288220daa442f), X(0x64648d6407e9c863), X(0xf1f1e3f1db12ff2a), X(0x7373d173bfa2e6cc), X(0x12124812905a2482),
            X(0x40401d403a5d807a), X(0x0808200840281048), X(0xc3c32bc356e89b95), X(0xecec97ec337bc5df), X(0xdbdb4bdb9690ab4d), X(0xa1a1bea1611f5fc0), X(0x8d8d0e8d1c830791), X(0x3d3df43df5c97ac8),
            X(0x97976697ccf1335b), X(0x0000000000000000), X(0xcfcf1bcf36d483f9), X(0x2b2bac2b4587566e), X(0x7676c57697b3ece1), X(0x8282328264b019e6), X(0xd6d67fd6fea9b128), X(0x1b1b6c1bd87736c3),
            X(0xb5b5eeb5c15b7774), X(0xafaf86af112943be), X(0x6a6ab56a77dfd41d), X(0x50505d50ba0da0ea), X(0x45450945124c8a57), X(0xf3f3ebf3cb18fb38), X(0x3030c0309df060ad), X(0xefef9bef2b74c3c4),
            X(0x3f3ffc3fe5c37eda), X(0x55554955921caac7), X(0xa2a2b2a2791059db), X(0xeaea8fea0365c9e9), X(0x656589650fecca6a), X(0xbabad2bab9686903), X(0x2f2fbc2f65935e4a), X(0xc0c027c04ee79d8e),
            X(0xdede5fdebe81a160), X(0x1c1c701ce06c38fc), X(0xfdfdd3fdbb2ee746), X(0x4d4d294d52649a1f), X(0x92927292e4e03976), X(0x7575c9758fbceafa), X(0x06061806301e0c36), X(0x8a8a128a249809ae),
            X(0xb2b2f2b2f940794b), X(0xe6e6bfe66359d185), X(0x0e0e380e70361c7e), X(0x1f1f7c1ff8633ee7), X(0x6262956237f7c455), X(0xd4d477d4eea3b53a), X(0xa8a89aa829324d81), X(0x96966296c4f43152),
            X(0xf9f9c3f99b3aef62), X(0xc5c533c566f697a3), X(0x2525942535b14a10), X(0x59597959f220b2ab), X(0x84842a8454ae15d0), X(0x7272d572b7a7e4c5), X(0x3939e439d5dd72ec), X(0x4c4c2d4c5a619816),
            X(0x5e5e655eca3bbc94), X(0x7878fd78e785f09f), X(0x3838e038ddd870e5), X(0x8c8c0a8c14860598), X(0xd1d163d1c6b2bf17), X(0xa5a5aea5410b57e4), X(0xe2e2afe2434dd9a1), X(0x616199612ff8c24e),
            X(0xb3b3f6b3f1457b42), X(0x2121842115a54234), X(0x9c9c4a9c94d62508), X(0x1e1e781ef0663cee), X(0x4343114322528661), X(0xc7c73bc776fc93b1), X(0xfcfcd7fcb32be54f), X(0x0404100420140824),
            X(0x51515951b208a2e3), X(0x99995e99bcc72f25), X(0x6d6da96d4fc4da22), X(0x0d0d340d68391a65), X(0xfafacffa8335e979), X(0xdfdf5bdfb684a369), X(0x7e7ee57ed79bfca9), X(0x242490243db44819),
            X(0x3b3bec3bc5d776fe), X(0xabab96ab313d4b9a), X(0xcece1fce3ed181f0), X(0x1111441188552299), X(0x8f8f068f0c890383), X(0x4e4e254e4a6b9c04), X(0xb7b7e6b7d1517366), X(0xebeb8beb0b60cbe0),
            X(0x3c3cf03cfdcc78c1), X(0x81813e817cbf1ffd), X(0x94946a94d4fe3540), X(0xf7f7fbf7eb0cf31c), X(0xb9b9deb9a1676f18), X(0x13134c13985f268b), X(0x2c2cb02c7d9c5851), X(0xd3d36bd3d6b8bb05),
            X(0xe7e7bbe76b5cd38c), X(0x6e6ea56e57cbdc39), X(0xc4c437c46ef395aa), X(0x03030c03180f061b), X(0x565645568a13acdc), X(0x44440d441a49885e), X(0x7f7fe17fdf9efea0), X(0xa9a99ea921374f88),
            X(0x2a2aa82a4d825467), X(0xbbbbd6bbb16d6b0a), X(0xc1c123c146e29f87), X(0x53535153a202a6f1), X(0xdcdc57dcae8ba572), X(0x0b0b2c0b58271653), X(0x9d9d4e9d9cd32701), X(0x6c6cad6c47c1d82b),
            X(0x3131c43195f562a4), X(0x7474cd7487b9e8f3), X(0xf6f6fff6e309f115), X(0x464605460a438c4c), X(0xacac8aac092645a5), X(0x89891e893c970fb5), X(0x14145014a04428b4), X(0xe1e1a3e15b42dfba),
            X(0x16165816b04e2ca6), X(0x3a3ae83acdd274f7), X(0x6969b9696fd0d206), X(0x09092409482d1241), X(0x7070dd70a7ade0d7), X(0xb6b6e2b6d954716f), X(0xd0d067d0ceb7bd1e), X(0xeded93ed3b7ec7d6),
            X(0xcccc17cc2edb85e2), X(0x424215422a578468), X(0x98985a98b4c22d2c), X(0xa4a4aaa4490e55ed), X(0x2828a0285d885075), X(0x5c5c6d5cda31b886), X(0xf8f8c7f8933fed6b), X(0x8686228644a411c2)
        };

        static const uint64 C1[] =
        {   X(0xd818186018c07830), X(0x2623238c2305af46), X(0xb8c6c63fc67ef991), X(0xfbe8e887e8136fcd), X(0xcb878726874ca113), X(0x11b8b8dab8a9626d), X(0x0901010401080502), X(0x0d4f4f214f426e9e),
            X(0x9b3636d836adee6c), X(0xffa6a6a2a6590451), X(0x0cd2d26fd2debdb9), X(0x0ef5f5f3f5fb06f7), X(0x967979f979ef80f2), X(0x306f6fa16f5fcede), X(0x6d91917e91fcef3f), X(0xf852525552aa07a4),
            X(0x4760609d6027fdc0), X(0x35bcbccabc897665), X(0x379b9b569baccd2b), X(0x8a8e8e028e048c01), X(0xd2a3a3b6a371155b), X(0x6c0c0c300c603c18), X(0x847b7bf17bff8af6), X(0x803535d435b5e16a),
            X(0xf51d1d741de8693a), X(0xb3e0e0a7e05347dd), X(0x21d7d77bd7f6acb3), X(0x9cc2c22fc25eed99), X(0x432e2eb82e6d965c), X(0x294b4b314b627a96), X(0x5dfefedffea321e1), X(0xd5575741578216ae),
            X(0xbd15155415a8412a), X(0xe87777c1779fb6ee), X(0x923737dc37a5eb6e), X(0x9ee5e5b3e57b56d7), X(0x139f9f469f8cd923), X(0x23f0f0e7f0d317fd), X(0x204a4a354a6a7f94), X(0x44dada4fda9e95a9),
            X(0xa258587d58fa25b0), X(0xcfc9c903c906ca8f), X(0x7c2929a429558d52), X(0x5a0a0a280a502214), X(0x50b1b1feb1e14f7f), X(0xc9a0a0baa0691a5d), X(0x146b6bb16b7fdad6), X(0xd985852e855cab17),
            X(0x3cbdbdcebd817367), X(0x8f5d5d695dd234ba), X(0x9010104010805020), X(0x07f4f4f7f4f303f5), X(0xddcbcb0bcb16c08b), X(0xd33e3ef83eedc67c), X(0x2d0505140528110a), X(0x78676781671fe6ce),
            X(0x97e4e4b7e47353d5), X(0x0227279c2725bb4e), X(0x7341411941325882), X(0xa78b8b168b2c9d0b), X(0xf6a7a7a6a7510153), X(0xb27d7de97dcf94fa), X(0x4995956e95dcfb37), X(0x56d8d847d88e9fad),
            X(0x70fbfbcbfb8b30eb), X(0xcdeeee9fee2371c1), X(0xbb7c7ced7cc791f8), X(0x716666856617e3cc), X(0x7bdddd53dda68ea7), X(0xaf17175c17b84b2e), X(0x454747014702468e), X(0x1a9e9e429e84dc21),
            X(0xd4caca0fca1ec589), X(0x582d2db42d75995a), X(0x2ebfbfc6bf917963), X(0x3f07071c07381b0e), X(0xacadad8ead012347), X(0xb05a5a755aea2fb4), X(0xef838336836cb51b), X(0xb63333cc3385ff66),
            X(0x5c636391633ff2c6), X(0x1202020802100a04), X(0x93aaaa92aa393849), X(0xde7171d971afa8e2), X(0xc6c8c807c80ecf8d), X(0xd119196419c87d32), X(0x3b49493949727092), X(0x5fd9d943d9869aaf),
            X(0x31f2f2eff2c31df9), X(0xa8e3e3abe34b48db), X(0xb95b5b715be22ab6), X(0xbc88881a8834920d), X(0x3e9a9a529aa4c829), X(0x0b262698262dbe4c), X(0xbf3232c8328dfa64), X(0x59b0b0fab0e94a7d),
            X(0xf2e9e983e91b6acf), X(0x770f0f3c0f78331e), X(0x33d5d573d5e6a6b7), X(0xf480803a8074ba1d), X(0x27bebec2be997c61), X(0xebcdcd13cd26de87), X(0x893434d034bde468), X(0x3248483d487a7590),
            X(0x54ffffdbffab24e3), X(0x8d7a7af57af78ff4), X(0x6490907a90f4ea3d), X(0x9d5f5f615fc23ebe), X(0x3d202080201da040), X(0x0f6868bd6867d5d0), X(0xca1a1a681ad07234), X(0xb7aeae82ae192c41),
            X(0x7db4b4eab4c95e75), X(0xce54544d549a19a8), X(0x7f93937693ece53b), X(0x2f222288220daa44), X(0x6364648d6407e9c8), X(0x2af1f1e3f1db12ff), X(0xcc7373d173bfa2e6), X(0x8212124812905a24),
            X(0x7a40401d403a5d80), X(0x4808082008402810), X(0x95c3c32bc356e89b), X(0xdfecec97ec337bc5), X(0x4ddbdb4bdb9690ab), X(0xc0a1a1bea1611f5f), X(0x918d8d0e8d1c8307), X(0xc83d3df43df5c97a),
            X(0x5b97976697ccf133), X(0x0000000000000000), X(0xf9cfcf1bcf36d483), X(0x6e2b2bac2b458756), X(0xe17676c57697b3ec), X(0xe68282328264b019), X(0x28d6d67fd6fea9b1), X(0xc31b1b6c1bd87736),
            X(0x74b5b5eeb5c15b77), X(0xbeafaf86af112943), X(0x1d6a6ab56a77dfd4), X(0xea50505d50ba0da0), X(0x5745450945124c8a), X(0x38f3f3ebf3cb18fb), X(0xad3030c0309df060), X(0xc4efef9bef2b74c3),
            X(0xda3f3ffc3fe5c37e), X(0xc755554955921caa), X(0xdba2a2b2a2791059), X(0xe9eaea8fea0365c9), X(0x6a656589650fecca), X(0x03babad2bab96869), X(0x4a2f2fbc2f65935e), X(0x8ec0c027c04ee79d),
            X(0x60dede5fdebe81a1), X(0xfc1c1c701ce06c38), X(0x46fdfdd3fdbb2ee7), X(0x1f4d4d294d52649a), X(0x7692927292e4e039), X(0xfa7575c9758fbcea), X(0x3606061806301e0c), X(0xae8a8a128a249809),
            X(0x4bb2b2f2b2f94079), X(0x85e6e6bfe66359d1), X(0x7e0e0e380e70361c), X(0xe71f1f7c1ff8633e), X(0x556262956237f7c4), X(0x3ad4d477d4eea3b5), X(0x81a8a89aa829324d), X(0x5296966296c4f431),
            X(0x62f9f9c3f99b3aef), X(0xa3c5c533c566f697), X(0x102525942535b14a), X(0xab59597959f220b2), X(0xd084842a8454ae15), X(0xc57272d572b7a7e4), X(0xec3939e439d5dd72), X(0x164c4c2d4c5a6198),
            X(0x945e5e655eca3bbc), X(0x9f7878fd78e785f0), X(0xe53838e038ddd870), X(0x988c8c0a8c148605), X(0x17d1d163d1c6b2bf), X(0xe4a5a5aea5410b57), X(0xa1e2e2afe2434dd9), X(0x4e616199612ff8c2),
            X(0x42b3b3f6b3f1457b), X(0x342121842115a542), X(0x089c9c4a9c94d625), X(0xee1e1e781ef0663c), X(0x6143431143225286), X(0xb1c7c73bc776fc93), X(0x4ffcfcd7fcb32be5), X(0x2404041004201408),
            X(0xe351515951b208a2), X(0x2599995e99bcc72f), X(0x226d6da96d4fc4da), X(0x650d0d340d68391a), X(0x79fafacffa8335e9), X(0x69dfdf5bdfb684a3), X(0xa97e7ee57ed79bfc), X(0x19242490243db448),
            X(0xfe3b3bec3bc5d776), X(0x9aabab96ab313d4b), X(0xf0cece1fce3ed181), X(0x9911114411885522), X(0x838f8f068f0c8903), X(0x044e4e254e4a6b9c), X(0x66b7b7e6b7d15173), X(0xe0ebeb8beb0b60cb),
            X(0xc13c3cf03cfdcc78), X(0xfd81813e817cbf1f), X(0x4094946a94d4fe35), X(0x1cf7f7fbf7eb0cf3), X(0x18b9b9deb9a1676f), X(0x8b13134c13985f26), X(0x512c2cb02c7d9c58), X(0x05d3d36bd3d6b8bb),
            X(0x8ce7e7bbe76b5cd3), X(0x396e6ea56e57cbdc), X(0xaac4c437c46ef395), X(0x1b03030c03180f06), X(0xdc565645568a13ac), X(0x5e44440d441a4988), X(0xa07f7fe17fdf9efe), X(0x88a9a99ea921374f),
            X(0x672a2aa82a4d8254), X(0x0abbbbd6bbb16d6b), X(0x87c1c123c146e29f), X(0xf153535153a202a6), X(0x72dcdc57dcae8ba5), X(0x530b0b2c0b582716), X(0x019d9d4e9d9cd327), X(0x2b6c6cad6c47c1d8),
            X(0xa43131c43195f562), X(0xf37474cd7487b9e8), X(0x15f6f6fff6e309f1), X(0x4c464605460a438c), X(0xa5acac8aac092645), X(0xb589891e893c970f), X(0xb414145014a04428), X(0xbae1e1a3e15b42df),
            X(0xa616165816b04e2c), X(0xf73a3ae83acdd274), X(0x066969b9696fd0d2), X(0x4109092409482d12), X(0xd77070dd70a7ade0), X(0x6fb6b6e2b6d95471), X(0x1ed0d067d0ceb7bd), X(0xd6eded93ed3b7ec7),
            X(0xe2cccc17cc2edb85), X(0x68424215422a5784), X(0x2c98985a98b4c22d), X(0xeda4a4aaa4490e55), X(0x752828a0285d8850), X(0x865c5c6d5cda31b8), X(0x6bf8f8c7f8933fed), X(0xc28686228644a411)
        };

        static const uint64 C2[] =
        {   X(0x30d818186018c078), X(0x462623238c2305af), X(0x91b8c6c63fc67ef9), X(0xcdfbe8e887e8136f), X(0x13cb878726874ca1), X(0x6d11b8b8dab8a962), X(0x0209010104010805), X(0x9e0d4f4f214f426e),
            X(0x6c9b3636d836adee), X(0x51ffa6a6a2a65904), X(0xb90cd2d26fd2debd), X(0xf70ef5f5f3f5fb06), X(0xf2967979f979ef80), X(0xde306f6fa16f5fce), X(0x3f6d91917e91fcef), X(0xa4f852525552aa07),
            X(0xc04760609d6027fd), X(0x6535bcbccabc8976), X(0x2b379b9b569baccd), X(0x018a8e8e028e048c), X(0x5bd2a3a3b6a37115), X(0x186c0c0c300c603c), X(0xf6847b7bf17bff8a), X(0x6a803535d435b5e1),
            X(0x3af51d1d741de869), X(0xddb3e0e0a7e05347), X(0xb321d7d77bd7f6ac), X(0x999cc2c22fc25eed), X(0x5c432e2eb82e6d96), X(0x96294b4b314b627a), X(0xe15dfefedffea321), X(0xaed5575741578216),
            X(0x2abd15155415a841), X(0xeee87777c1779fb6), X(0x6e923737dc37a5eb), X(0xd79ee5e5b3e57b56), X(0x23139f9f469f8cd9), X(0xfd23f0f0e7f0d317), X(0x94204a4a354a6a7f), X(0xa944dada4fda9e95),
            X(0xb0a258587d58fa25), X(0x8fcfc9c903c906ca), X(0x527c2929a429558d), X(0x145a0a0a280a5022), X(0x7f50b1b1feb1e14f), X(0x5dc9a0a0baa0691a), X(0xd6146b6bb16b7fda), X(0x17d985852e855cab),
            X(0x673cbdbdcebd8173), X(0xba8f5d5d695dd234), X(0x2090101040108050), X(0xf507f4f4f7f4f303), X(0x8bddcbcb0bcb16c0), X(0x7cd33e3ef83eedc6), X(0x0a2d050514052811), X(0xce78676781671fe6),
            X(0xd597e4e4b7e47353), X(0x4e0227279c2725bb), X(0x8273414119413258), X(0x0ba78b8b168b2c9d), X(0x53f6a7a7a6a75101), X(0xfab27d7de97dcf94), X(0x374995956e95dcfb), X(0xad56d8d847d88e9f),
            X(0xeb70fbfbcbfb8b30), X(0xc1cdeeee9fee2371), X(0xf8bb7c7ced7cc791), X(0xcc716666856617e3), X(0xa77bdddd53dda68e), X(0x2eaf17175c17b84b), X(0x8e45474701470246), X(0x211a9e9e429e84dc),
            X(0x89d4caca0fca1ec5), X(0x5a582d2db42d7599), X(0x632ebfbfc6bf9179), X(0x0e3f07071c07381b), X(0x47acadad8ead0123), X(0xb4b05a5a755aea2f), X(0x1bef838336836cb5), X(0x66b63333cc3385ff),
            X(0xc65c636391633ff2), X(0x041202020802100a), X(0x4993aaaa92aa3938), X(0xe2de7171d971afa8), X(0x8dc6c8c807c80ecf), X(0x32d119196419c87d), X(0x923b494939497270), X(0xaf5fd9d943d9869a),
            X(0xf931f2f2eff2c31d), X(0xdba8e3e3abe34b48), X(0xb6b95b5b715be22a), X(0x0dbc88881a883492), X(0x293e9a9a529aa4c8), X(0x4c0b262698262dbe), X(0x64bf3232c8328dfa), X(0x7d59b0b0fab0e94a),
            X(0xcff2e9e983e91b6a), X(0x1e770f0f3c0f7833), X(0xb733d5d573d5e6a6), X(0x1df480803a8074ba), X(0x6127bebec2be997c), X(0x87ebcdcd13cd26de), X(0x68893434d034bde4), X(0x903248483d487a75),
            X(0xe354ffffdbffab24), X(0xf48d7a7af57af78f), X(0x3d6490907a90f4ea), X(0xbe9d5f5f615fc23e), X(0x403d202080201da0), X(0xd00f6868bd6867d5), X(0x34ca1a1a681ad072), X(0x41b7aeae82ae192c),
            X(0x757db4b4eab4c95e), X(0xa8ce54544d549a19), X(0x3b7f93937693ece5), X(0x442f222288220daa), X(0xc86364648d6407e9), X(0xff2af1f1e3f1db12), X(0xe6cc7373d173bfa2), X(0x248212124812905a),
            X(0x807a40401d403a5d), X(0x1048080820084028), X(0x9b95c3c32bc356e8), X(0xc5dfecec97ec337b), X(0xab4ddbdb4bdb9690), X(0x5fc0a1a1bea1611f), X(0x07918d8d0e8d1c83), X(0x7ac83d3df43df5c9),
            X(0x335b97976697ccf1), X(0x0000000000000000), X(0x83f9cfcf1bcf36d4), X(0x566e2b2bac2b4587), X(0xece17676c57697b3), X(0x19e68282328264b0), X(0xb128d6d67fd6fea9), X(0x36c31b1b6c1bd877),
            X(0x7774b5b5eeb5c15b), X(0x43beafaf86af1129), X(0xd41d6a6ab56a77df), X(0xa0ea50505d50ba0d), X(0x8a5745450945124c), X(0xfb38f3f3ebf3cb18), X(0x60ad3030c0309df0), X(0xc3c4efef9bef2b74),
            X(0x7eda3f3ffc3fe5c3), X(0xaac755554955921c), X(0x59dba2a2b2a27910), X(0xc9e9eaea8fea0365), X(0xca6a656589650fec), X(0x6903babad2bab968), X(0x5e4a2f2fbc2f6593), X(0x9d8ec0c027c04ee7),
            X(0xa160dede5fdebe81), X(0x38fc1c1c701ce06c), X(0xe746fdfdd3fdbb2e), X(0x9a1f4d4d294d5264), X(0x397692927292e4e0), X(0xeafa7575c9758fbc), X(0x0c3606061806301e), X(0x09ae8a8a128a2498),
            X(0x794bb2b2f2b2f940), X(0xd185e6e6bfe66359), X(0x1c7e0e0e380e7036), X(0x3ee71f1f7c1ff863), X(0xc4556262956237f7), X(0xb53ad4d477d4eea3), X(0x4d81a8a89aa82932), X(0x315296966296c4f4),
            X(0xef62f9f9c3f99b3a), X(0x97a3c5c533c566f6), X(0x4a102525942535b1), X(0xb2ab59597959f220), X(0x15d084842a8454ae), X(0xe4c57272d572b7a7), X(0x72ec3939e439d5dd), X(0x98164c4c2d4c5a61),
            X(0xbc945e5e655eca3b), X(0xf09f7878fd78e785), X(0x70e53838e038ddd8), X(0x05988c8c0a8c1486), X(0xbf17d1d163d1c6b2), X(0x57e4a5a5aea5410b), X(0xd9a1e2e2afe2434d), X(0xc24e616199612ff8),
            X(0x7b42b3b3f6b3f145), X(0x42342121842115a5), X(0x25089c9c4a9c94d6), X(0x3cee1e1e781ef066), X(0x8661434311432252), X(0x93b1c7c73bc776fc), X(0xe54ffcfcd7fcb32b), X(0x0824040410042014),
            X(0xa2e351515951b208), X(0x2f2599995e99bcc7), X(0xda226d6da96d4fc4), X(0x1a650d0d340d6839), X(0xe979fafacffa8335), X(0xa369dfdf5bdfb684), X(0xfca97e7ee57ed79b), X(0x4819242490243db4),
            X(0x76fe3b3bec3bc5d7), X(0x4b9aabab96ab313d), X(0x81f0cece1fce3ed1), X(0x2299111144118855), X(0x03838f8f068f0c89), X(0x9c044e4e254e4a6b), X(0x7366b7b7e6b7d151), X(0xcbe0ebeb8beb0b60),
            X(0x78c13c3cf03cfdcc), X(0x1ffd81813e817cbf), X(0x354094946a94d4fe), X(0xf31cf7f7fbf7eb0c), X(0x6f18b9b9deb9a167), X(0x268b13134c13985f), X(0x58512c2cb02c7d9c), X(0xbb05d3d36bd3d6b8),
            X(0xd38ce7e7bbe76b5c), X(0xdc396e6ea56e57cb), X(0x95aac4c437c46ef3), X(0x061b03030c03180f), X(0xacdc565645568a13), X(0x885e44440d441a49), X(0xfea07f7fe17fdf9e), X(0x4f88a9a99ea92137),
            X(0x54672a2aa82a4d82), X(0x6b0abbbbd6bbb16d), X(0x9f87c1c123c146e2), X(0xa6f153535153a202), X(0xa572dcdc57dcae8b), X(0x16530b0b2c0b5827), X(0x27019d9d4e9d9cd3), X(0xd82b6c6cad6c47c1),
            X(0x62a43131c43195f5), X(0xe8f37474cd7487b9), X(0xf115f6f6fff6e309), X(0x8c4c464605460a43), X(0x45a5acac8aac0926), X(0x0fb589891e893c97), X(0x28b414145014a044), X(0xdfbae1e1a3e15b42),
            X(0x2ca616165816b04e), X(0x74f73a3ae83acdd2), X(0xd2066969b9696fd0), X(0x124109092409482d), X(0xe0d77070dd70a7ad), X(0x716fb6b6e2b6d954), X(0xbd1ed0d067d0ceb7), X(0xc7d6eded93ed3b7e),
            X(0x85e2cccc17cc2edb), X(0x8468424215422a57), X(0x2d2c98985a98b4c2), X(0x55eda4a4aaa4490e), X(0x50752828a0285d88), X(0xb8865c5c6d5cda31), X(0xed6bf8f8c7f8933f), X(0x11c28686228644a4)
        };

        static const uint64 C3[] =
        {   X(0x7830d818186018c0), X(0xaf462623238c2305), X(0xf991b8c6c63fc67e), X(0x6fcdfbe8e887e813), X(0xa113cb878726874c), X(0x626d11b8b8dab8a9), X(0x0502090101040108), X(0x6e9e0d4f4f214f42),
            X(0xee6c9b3636d836ad), X(0x0451ffa6a6a2a659), X(0xbdb90cd2d26fd2de), X(0x06f70ef5f5f3f5fb), X(0x80f2967979f979ef), X(0xcede306f6fa16f5f), X(0xef3f6d91917e91fc), X(0x07a4f852525552aa),
            X(0xfdc04760609d6027), X(0x766535bcbccabc89), X(0xcd2b379b9b569bac), X(0x8c018a8e8e028e04), X(0x155bd2a3a3b6a371), X(0x3c186c0c0c300c60), X(0x8af6847b7bf17bff), X(0xe16a803535d435b5),
            X(0x693af51d1d741de8), X(0x47ddb3e0e0a7e053), X(0xacb321d7d77bd7f6), X(0xed999cc2c22fc25e), X(0x965c432e2eb82e6d), X(0x7a96294b4b314b62), X(0x21e15dfefedffea3), X(0x16aed55757415782),
            X(0x412abd15155415a8), X(0xb6eee87777c1779f), X(0xeb6e923737dc37a5), X(0x56d79ee5e5b3e57b), X(0xd923139f9f469f8c), X(0x17fd23f0f0e7f0d3), X(0x7f94204a4a354a6a), X(0x95a944dada4fda9e),
            X(0x25b0a258587d58fa), X(0xca8fcfc9c903c906), X(0x8d527c2929a42955), X(0x22145a0a0a280a50), X(0x4f7f50b1b1feb1e1), X(0x1a5dc9a0a0baa069), X(0xdad6146b6bb16b7f), X(0xab17d985852e855c),
            X(0x73673cbdbdcebd81), X(0x34ba8f5d5d695dd2), X(0x5020901010401080), X(0x03f507f4f4f7f4f3), X(0xc08bddcbcb0bcb16), X(0xc67cd33e3ef83eed), X(0x110a2d0505140528), X(0xe6ce78676781671f),
            X(0x53d597e4e4b7e473), X(0xbb4e0227279c2725), X(0x5882734141194132), X(0x9d0ba78b8b168b2c), X(0x0153f6a7a7a6a751), X(0x94fab27d7de97dcf), X(0xfb374995956e95dc), X(0x9fad56d8d847d88e),
            X(0x30eb70fbfbcbfb8b), X(0x71c1cdeeee9fee23), X(0x91f8bb7c7ced7cc7), X(0xe3cc716666856617), X(0x8ea77bdddd53dda6), X(0x4b2eaf17175c17b8), X(0x468e454747014702), X(0xdc211a9e9e429e84),
            X(0xc589d4caca0fca1e), X(0x995a582d2db42d75), X(0x79632ebfbfc6bf91), X(0x1b0e3f07071c0738), X(0x2347acadad8ead01), X(0x2fb4b05a5a755aea), X(0xb51bef838336836c), X(0xff66b63333cc3385),
            X(0xf2c65c636391633f), X(0x0a04120202080210), X(0x384993aaaa92aa39), X(0xa8e2de7171d971af), X(0xcf8dc6c8c807c80e), X(0x7d32d119196419c8), X(0x70923b4949394972), X(0x9aaf5fd9d943d986),
            X(0x1df931f2f2eff2c3), X(0x48dba8e3e3abe34b), X(0x2ab6b95b5b715be2), X(0x920dbc88881a8834), X(0xc8293e9a9a529aa4), X(0xbe4c0b262698262d), X(0xfa64bf3232c8328d), X(0x4a7d59b0b0fab0e9),
            X(0x6acff2e9e983e91b), X(0x331e770f0f3c0f78), X(0xa6b733d5d573d5e6), X(0xba1df480803a8074), X(0x7c6127bebec2be99), X(0xde87ebcdcd13cd26), X(0xe468893434d034bd), X(0x75903248483d487a),
            X(0x24e354ffffdbffab), X(0x8ff48d7a7af57af7), X(0xea3d6490907a90f4), X(0x3ebe9d5f5f615fc2), X(0xa0403d202080201d), X(0xd5d00f6868bd6867), X(0x7234ca1a1a681ad0), X(0x2c41b7aeae82ae19),
            X(0x5e757db4b4eab4c9), X(0x19a8ce54544d549a), X(0xe53b7f93937693ec), X(0xaa442f222288220d), X(0xe9c86364648d6407), X(0x12ff2af1f1e3f1db), X(0xa2e6cc7373d173bf), X(0x5a24821212481290),
            X(0x5d807a40401d403a), X(0x2810480808200840), X(0xe89b95c3c32bc356), X(0x7bc5dfecec97ec33), X(0x90ab4ddbdb4bdb96), X(0x1f5fc0a1a1bea161), X(0x8307918d8d0e8d1c), X(0xc97ac83d3df43df5),
            X(0xf1335b97976697cc), X(0x0000000000000000), X(0xd483f9cfcf1bcf36), X(0x87566e2b2bac2b45), X(0xb3ece17676c57697), X(0xb019e68282328264), X(0xa9b128d6d67fd6fe), X(0x7736c31b1b6c1bd8),
            X(0x5b7774b5b5eeb5c1), X(0x2943beafaf86af11), X(0xdfd41d6a6ab56a77), X(0x0da0ea50505d50ba), X(0x4c8a574545094512), X(0x18fb38f3f3ebf3cb), X(0xf060ad3030c0309d), X(0x74c3c4efef9bef2b),
            X(0xc37eda3f3ffc3fe5), X(0x1caac75555495592), X(0x1059dba2a2b2a279), X(0x65c9e9eaea8fea03), X(0xecca6a656589650f), X(0x686903babad2bab9), X(0x935e4a2f2fbc2f65), X(0xe79d8ec0c027c04e),
            X(0x81a160dede5fdebe), X(0x6c38fc1c1c701ce0), X(0x2ee746fdfdd3fdbb), X(0x649a1f4d4d294d52), X(0xe0397692927292e4), X(0xbceafa7575c9758f), X(0x1e0c360606180630), X(0x9809ae8a8a128a24),
            X(0x40794bb2b2f2b2f9), X(0x59d185e6e6bfe663), X(0x361c7e0e0e380e70), X(0x633ee71f1f7c1ff8), X(0xf7c4556262956237), X(0xa3b53ad4d477d4ee), X(0x324d81a8a89aa829), X(0xf4315296966296c4),
            X(0x3aef62f9f9c3f99b), X(0xf697a3c5c533c566), X(0xb14a102525942535), X(0x20b2ab59597959f2), X(0xae15d084842a8454), X(0xa7e4c57272d572b7), X(0xdd72ec3939e439d5), X(0x6198164c4c2d4c5a),
            X(0x3bbc945e5e655eca), X(0x85f09f7878fd78e7), X(0xd870e53838e038dd), X(0x8605988c8c0a8c14), X(0xb2bf17d1d163d1c6), X(0x0b57e4a5a5aea541), X(0x4dd9a1e2e2afe243), X(0xf8c24e616199612f),
            X(0x457b42b3b3f6b3f1), X(0xa542342121842115), X(0xd625089c9c4a9c94), X(0x663cee1e1e781ef0), X(0x5286614343114322), X(0xfc93b1c7c73bc776), X(0x2be54ffcfcd7fcb3), X(0x1408240404100420),
            X(0x08a2e351515951b2), X(0xc72f2599995e99bc), X(0xc4da226d6da96d4f), X(0x391a650d0d340d68), X(0x35e979fafacffa83), X(0x84a369dfdf5bdfb6), X(0x9bfca97e7ee57ed7), X(0xb44819242490243d),
            X(0xd776fe3b3bec3bc5), X(0x3d4b9aabab96ab31), X(0xd181f0cece1fce3e), X(0x5522991111441188), X(0x8903838f8f068f0c), X(0x6b9c044e4e254e4a), X(0x517366b7b7e6b7d1), X(0x60cbe0ebeb8beb0b),
            X(0xcc78c13c3cf03cfd), X(0xbf1ffd81813e817c), X(0xfe354094946a94d4), X(0x0cf31cf7f7fbf7eb), X(0x676f18b9b9deb9a1), X(0x5f268b13134c1398), X(0x9c58512c2cb02c7d), X(0xb8bb05d3d36bd3d6),
            X(0x5cd38ce7e7bbe76b), X(0xcbdc396e6ea56e57), X(0xf395aac4c437c46e), X(0x0f061b03030c0318), X(0x13acdc565645568a), X(0x49885e44440d441a), X(0x9efea07f7fe17fdf), X(0x374f88a9a99ea921),
            X(0x8254672a2aa82a4d), X(0x6d6b0abbbbd6bbb1), X(0xe29f87c1c123c146), X(0x02a6f153535153a2), X(0x8ba572dcdc57dcae), X(0x2716530b0b2c0b58), X(0xd327019d9d4e9d9c), X(0xc1d82b6c6cad6c47),
            X(0xf562a43131c43195), X(0xb9e8f37474cd7487), X(0x09f115f6f6fff6e3), X(0x438c4c464605460a), X(0x2645a5acac8aac09), X(0x970fb589891e893c), X(0x4428b414145014a0), X(0x42dfbae1e1a3e15b),
            X(0x4e2ca616165816b0), X(0xd274f73a3ae83acd), X(0xd0d2066969b9696f), X(0x2d12410909240948), X(0xade0d77070dd70a7), X(0x54716fb6b6e2b6d9), X(0xb7bd1ed0d067d0ce), X(0x7ec7d6eded93ed3b),
            X(0xdb85e2cccc17cc2e), X(0x578468424215422a), X(0xc22d2c98985a98b4), X(0x0e55eda4a4aaa449), X(0x8850752828a0285d), X(0x31b8865c5c6d5cda), X(0x3fed6bf8f8c7f893), X(0xa411c28686228644)
        };

        static const uint64 C4[] =
        {   X(0xc07830d818186018), X(0x05af462623238c23), X(0x7ef991b8c6c63fc6), X(0x136fcdfbe8e887e8), X(0x4ca113cb87872687), X(0xa9626d11b8b8dab8), X(0x0805020901010401), X(0x426e9e0d4f4f214f),
            X(0xadee6c9b3636d836), X(0x590451ffa6a6a2a6), X(0xdebdb90cd2d26fd2), X(0xfb06f70ef5f5f3f5), X(0xef80f2967979f979), X(0x5fcede306f6fa16f), X(0xfcef3f6d91917e91), X(0xaa07a4f852525552),
            X(0x27fdc04760609d60), X(0x89766535bcbccabc), X(0xaccd2b379b9b569b), X(0x048c018a8e8e028e), X(0x71155bd2a3a3b6a3), X(0x603c186c0c0c300c), X(0xff8af6847b7bf17b), X(0xb5e16a803535d435),
            X(0xe8693af51d1d741d), X(0x5347ddb3e0e0a7e0), X(0xf6acb321d7d77bd7), X(0x5eed999cc2c22fc2), X(0x6d965c432e2eb82e), X(0x627a96294b4b314b), X(0xa321e15dfefedffe), X(0x8216aed557574157),
            X(0xa8412abd15155415), X(0x9fb6eee87777c177), X(0xa5eb6e923737dc37), X(0x7b56d79ee5e5b3e5), X(0x8cd923139f9f469f), X(0xd317fd23f0f0e7f0), X(0x6a7f94204a4a354a), X(0x9e95a944dada4fda),
            X(0xfa25b0a258587d58), X(0x06ca8fcfc9c903c9), X(0x558d527c2929a429), X(0x5022145a0a0a280a), X(0xe14f7f50b1b1feb1), X(0x691a5dc9a0a0baa0), X(0x7fdad6146b6bb16b), X(0x5cab17d985852e85),
            X(0x8173673cbdbdcebd), X(0xd234ba8f5d5d695d), X(0x8050209010104010), X(0xf303f507f4f4f7f4), X(0x16c08bddcbcb0bcb), X(0xedc67cd33e3ef83e), X(0x28110a2d05051405), X(0x1fe6ce7867678167),
            X(0x7353d597e4e4b7e4), X(0x25bb4e0227279c27), X(0x3258827341411941), X(0x2c9d0ba78b8b168b), X(0x510153f6a7a7a6a7), X(0xcf94fab27d7de97d), X(0xdcfb374995956e95), X(0x8e9fad56d8d847d8),
            X(0x8b30eb70fbfbcbfb), X(0x2371c1cdeeee9fee), X(0xc791f8bb7c7ced7c), X(0x17e3cc7166668566), X(0xa68ea77bdddd53dd), X(0xb84b2eaf17175c17), X(0x02468e4547470147), X(0x84dc211a9e9e429e),
            X(0x1ec589d4caca0fca), X(0x75995a582d2db42d), X(0x9179632ebfbfc6bf), X(0x381b0e3f07071c07), X(0x012347acadad8ead), X(0xea2fb4b05a5a755a), X(0x6cb51bef83833683), X(0x85ff66b63333cc33),
            X(0x3ff2c65c63639163), X(0x100a041202020802), X(0x39384993aaaa92aa), X(0xafa8e2de7171d971), X(0x0ecf8dc6c8c807c8), X(0xc87d32d119196419), X(0x7270923b49493949), X(0x869aaf5fd9d943d9),
            X(0xc31df931f2f2eff2), X(0x4b48dba8e3e3abe3), X(0xe22ab6b95b5b715b), X(0x34920dbc88881a88), X(0xa4c8293e9a9a529a), X(0x2dbe4c0b26269826), X(0x8dfa64bf3232c832), X(0xe94a7d59b0b0fab0),
            X(0x1b6acff2e9e983e9), X(0x78331e770f0f3c0f), X(0xe6a6b733d5d573d5), X(0x74ba1df480803a80), X(0x997c6127bebec2be), X(0x26de87ebcdcd13cd), X(0xbde468893434d034), X(0x7a75903248483d48),
            X(0xab24e354ffffdbff), X(0xf78ff48d7a7af57a), X(0xf4ea3d6490907a90), X(0xc23ebe9d5f5f615f), X(0x1da0403d20208020), X(0x67d5d00f6868bd68), X(0xd07234ca1a1a681a), X(0x192c41b7aeae82ae),
            X(0xc95e757db4b4eab4), X(0x9a19a8ce54544d54), X(0xece53b7f93937693), X(0x0daa442f22228822), X(0x07e9c86364648d64), X(0xdb12ff2af1f1e3f1), X(0xbfa2e6cc7373d173), X(0x905a248212124812),
            X(0x3a5d807a40401d40), X(0x4028104808082008), X(0x56e89b95c3c32bc3), X(0x337bc5dfecec97ec), X(0x9690ab4ddbdb4bdb), X(0x611f5fc0a1a1bea1), X(0x1c8307918d8d0e8d), X(0xf5c97ac83d3df43d),
            X(0xccf1335b97976697), X(0x0000000000000000), X(0x36d483f9cfcf1bcf), X(0x4587566e2b2bac2b), X(0x97b3ece17676c576), X(0x64b019e682823282), X(0xfea9b128d6d67fd6), X(0xd87736c31b1b6c1b),
            X(0xc15b7774b5b5eeb5), X(0x112943beafaf86af), X(0x77dfd41d6a6ab56a), X(0xba0da0ea50505d50), X(0x124c8a5745450945), X(0xcb18fb38f3f3ebf3), X(0x9df060ad3030c030), X(0x2b74c3c4efef9bef),
            X(0xe5c37eda3f3ffc3f), X(0x921caac755554955), X(0x791059dba2a2b2a2), X(0x0365c9e9eaea8fea), X(0x0fecca6a65658965), X(0xb9686903babad2ba), X(0x65935e4a2f2fbc2f), X(0x4ee79d8ec0c027c0),
            X(0xbe81a160dede5fde), X(0xe06c38fc1c1c701c), X(0xbb2ee746fdfdd3fd), X(0x52649a1f4d4d294d), X(0xe4e0397692927292), X(0x8fbceafa7575c975), X(0x301e0c3606061806), X(0x249809ae8a8a128a),
            X(0xf940794bb2b2f2b2), X(0x6359d185e6e6bfe6), X(0x70361c7e0e0e380e), X(0xf8633ee71f1f7c1f), X(0x37f7c45562629562), X(0xeea3b53ad4d477d4), X(0x29324d81a8a89aa8), X(0xc4f4315296966296),
            X(0x9b3aef62f9f9c3f9), X(0x66f697a3c5c533c5), X(0x35b14a1025259425), X(0xf220b2ab59597959), X(0x54ae15d084842a84), X(0xb7a7e4c57272d572), X(0xd5dd72ec3939e439), X(0x5a6198164c4c2d4c),
            X(0xca3bbc945e5e655e), X(0xe785f09f7878fd78), X(0xddd870e53838e038), X(0x148605988c8c0a8c), X(0xc6b2bf17d1d163d1), X(0x410b57e4a5a5aea5), X(0x434dd9a1e2e2afe2), X(0x2ff8c24e61619961),
            X(0xf1457b42b3b3f6b3), X(0x15a5423421218421), X(0x94d625089c9c4a9c), X(0xf0663cee1e1e781e), X(0x2252866143431143), X(0x76fc93b1c7c73bc7), X(0xb32be54ffcfcd7fc), X(0x2014082404041004),
            X(0xb208a2e351515951), X(0xbcc72f2599995e99), X(0x4fc4da226d6da96d), X(0x68391a650d0d340d), X(0x8335e979fafacffa), X(0xb684a369dfdf5bdf), X(0xd79bfca97e7ee57e), X(0x3db4481924249024),
            X(0xc5d776fe3b3bec3b), X(0x313d4b9aabab96ab), X(0x3ed181f0cece1fce), X(0x8855229911114411), X(0x0c8903838f8f068f), X(0x4a6b9c044e4e254e), X(0xd1517366b7b7e6b7), X(0x0b60cbe0ebeb8beb),
            X(0xfdcc78c13c3cf03c), X(0x7cbf1ffd81813e81), X(0xd4fe354094946a94), X(0xeb0cf31cf7f7fbf7), X(0xa1676f18b9b9deb9), X(0x985f268b13134c13), X(0x7d9c58512c2cb02c), X(0xd6b8bb05d3d36bd3),
            X(0x6b5cd38ce7e7bbe7), X(0x57cbdc396e6ea56e), X(0x6ef395aac4c437c4), X(0x180f061b03030c03), X(0x8a13acdc56564556), X(0x1a49885e44440d44), X(0xdf9efea07f7fe17f), X(0x21374f88a9a99ea9),
            X(0x4d8254672a2aa82a), X(0xb16d6b0abbbbd6bb), X(0x46e29f87c1c123c1), X(0xa202a6f153535153), X(0xae8ba572dcdc57dc), X(0x582716530b0b2c0b), X(0x9cd327019d9d4e9d), X(0x47c1d82b6c6cad6c),
            X(0x95f562a43131c431), X(0x87b9e8f37474cd74), X(0xe309f115f6f6fff6), X(0x0a438c4c46460546), X(0x092645a5acac8aac), X(0x3c970fb589891e89), X(0xa04428b414145014), X(0x5b42dfbae1e1a3e1),
            X(0xb04e2ca616165816), X(0xcdd274f73a3ae83a), X(0x6fd0d2066969b969), X(0x482d124109092409), X(0xa7ade0d77070dd70), X(0xd954716fb6b6e2b6), X(0xceb7bd1ed0d067d0), X(0x3b7ec7d6eded93ed),
            X(0x2edb85e2cccc17cc), X(0x2a57846842421542), X(0xb4c22d2c98985a98), X(0x490e55eda4a4aaa4), X(0x5d8850752828a028), X(0xda31b8865c5c6d5c), X(0x933fed6bf8f8c7f8), X(0x44a411c286862286)
        };

        static const uint64 C5[] =
        {   X(0x18c07830d8181860), X(0x2305af462623238c), X(0xc67ef991b8c6c63f), X(0xe8136fcdfbe8e887), X(0x874ca113cb878726), X(0xb8a9626d11b8b8da), X(0x0108050209010104), X(0x4f426e9e0d4f4f21),
            X(0x36adee6c9b3636d8), X(0xa6590451ffa6a6a2), X(0xd2debdb90cd2d26f), X(0xf5fb06f70ef5f5f3), X(0x79ef80f2967979f9), X(0x6f5fcede306f6fa1), X(0x91fcef3f6d91917e), X(0x52aa07a4f8525255),
            X(0x6027fdc04760609d), X(0xbc89766535bcbcca), X(0x9baccd2b379b9b56), X(0x8e048c018a8e8e02), X(0xa371155bd2a3a3b6), X(0x0c603c186c0c0c30), X(0x7bff8af6847b7bf1), X(0x35b5e16a803535d4),
            X(0x1de8693af51d1d74), X(0xe05347ddb3e0e0a7), X(0xd7f6acb321d7d77b), X(0xc25eed999cc2c22f), X(0x2e6d965c432e2eb8), X(0x4b627a96294b4b31), X(0xfea321e15dfefedf), X(0x578216aed5575741),
            X(0x15a8412abd151554), X(0x779fb6eee87777c1), X(0x37a5eb6e923737dc), X(0xe57b56d79ee5e5b3), X(0x9f8cd923139f9f46), X(0xf0d317fd23f0f0e7), X(0x4a6a7f94204a4a35), X(0xda9e95a944dada4f),
            X(0x58fa25b0a258587d), X(0xc906ca8fcfc9c903), X(0x29558d527c2929a4), X(0x0a5022145a0a0a28), X(0xb1e14f7f50b1b1fe), X(0xa0691a5dc9a0a0ba), X(0x6b7fdad6146b6bb1), X(0x855cab17d985852e),
            X(0xbd8173673cbdbdce), X(0x5dd234ba8f5d5d69), X(0x1080502090101040), X(0xf4f303f507f4f4f7), X(0xcb16c08bddcbcb0b), X(0x3eedc67cd33e3ef8), X(0x0528110a2d050514), X(0x671fe6ce78676781),
            X(0xe47353d597e4e4b7), X(0x2725bb4e0227279c), X(0x4132588273414119), X(0x8b2c9d0ba78b8b16), X(0xa7510153f6a7a7a6), X(0x7dcf94fab27d7de9), X(0x95dcfb374995956e), X(0xd88e9fad56d8d847),
            X(0xfb8b30eb70fbfbcb), X(0xee2371c1cdeeee9f), X(0x7cc791f8bb7c7ced), X(0x6617e3cc71666685), X(0xdda68ea77bdddd53), X(0x17b84b2eaf17175c), X(0x4702468e45474701), X(0x9e84dc211a9e9e42),
            X(0xca1ec589d4caca0f), X(0x2d75995a582d2db4), X(0xbf9179632ebfbfc6), X(0x07381b0e3f07071c), X(0xad012347acadad8e), X(0x5aea2fb4b05a5a75), X(0x836cb51bef838336), X(0x3385ff66b63333cc),
            X(0x633ff2c65c636391), X(0x02100a0412020208), X(0xaa39384993aaaa92), X(0x71afa8e2de7171d9), X(0xc80ecf8dc6c8c807), X(0x19c87d32d1191964), X(0x497270923b494939), X(0xd9869aaf5fd9d943),
            X(0xf2c31df931f2f2ef), X(0xe34b48dba8e3e3ab), X(0x5be22ab6b95b5b71), X(0x8834920dbc88881a), X(0x9aa4c8293e9a9a52), X(0x262dbe4c0b262698), X(0x328dfa64bf3232c8), X(0xb0e94a7d59b0b0fa),
            X(0xe91b6acff2e9e983), X(0x0f78331e770f0f3c), X(0xd5e6a6b733d5d573), X(0x8074ba1df480803a), X(0xbe997c6127bebec2), X(0xcd26de87ebcdcd13), X(0x34bde468893434d0), X(0x487a75903248483d),
            X(0xffab24e354ffffdb), X(0x7af78ff48d7a7af5), X(0x90f4ea3d6490907a), X(0x5fc23ebe9d5f5f61), X(0x201da0403d202080), X(0x6867d5d00f6868bd), X(0x1ad07234ca1a1a68), X(0xae192c41b7aeae82),
            X(0xb4c95e757db4b4ea), X(0x549a19a8ce54544d), X(0x93ece53b7f939376), X(0x220daa442f222288), X(0x6407e9c86364648d), X(0xf1db12ff2af1f1e3), X(0x73bfa2e6cc7373d1), X(0x12905a2482121248),
            X(0x403a5d807a40401d), X(0x0840281048080820), X(0xc356e89b95c3c32b), X(0xec337bc5dfecec97), X(0xdb9690ab4ddbdb4b), X(0xa1611f5fc0a1a1be), X(0x8d1c8307918d8d0e), X(0x3df5c97ac83d3df4),
            X(0x97ccf1335b979766), X(0x0000000000000000), X(0xcf36d483f9cfcf1b), X(0x2b4587566e2b2bac), X(0x7697b3ece17676c5), X(0x8264b019e6828232), X(0xd6fea9b128d6d67f), X(0x1bd87736c31b1b6c),
            X(0xb5c15b7774b5b5ee), X(0xaf112943beafaf86), X(0x6a77dfd41d6a6ab5), X(0x50ba0da0ea50505d), X(0x45124c8a57454509), X(0xf3cb18fb38f3f3eb), X(0x309df060ad3030c0), X(0xef2b74c3c4efef9b),
            X(0x3fe5c37eda3f3ffc), X(0x55921caac7555549), X(0xa2791059dba2a2b2), X(0xea0365c9e9eaea8f), X(0x650fecca6a656589), X(0xbab9686903babad2), X(0x2f65935e4a2f2fbc), X(0xc04ee79d8ec0c027),
            X(0xdebe81a160dede5f), X(0x1ce06c38fc1c1c70), X(0xfdbb2ee746fdfdd3), X(0x4d52649a1f4d4d29), X(0x92e4e03976929272), X(0x758fbceafa7575c9), X(0x06301e0c36060618), X(0x8a249809ae8a8a12),
            X(0xb2f940794bb2b2f2), X(0xe66359d185e6e6bf), X(0x0e70361c7e0e0e38), X(0x1ff8633ee71f1f7c), X(0x6237f7c455626295), X(0xd4eea3b53ad4d477), X(0xa829324d81a8a89a), X(0x96c4f43152969662),
            X(0xf99b3aef62f9f9c3), X(0xc566f697a3c5c533), X(0x2535b14a10252594), X(0x59f220b2ab595979), X(0x8454ae15d084842a), X(0x72b7a7e4c57272d5), X(0x39d5dd72ec3939e4), X(0x4c5a6198164c4c2d),
            X(0x5eca3bbc945e5e65), X(0x78e785f09f7878fd), X(0x38ddd870e53838e0), X(0x8c148605988c8c0a), X(0xd1c6b2bf17d1d163), X(0xa5410b57e4a5a5ae), X(0xe2434dd9a1e2e2af), X(0x612ff8c24e616199),
            X(0xb3f1457b42b3b3f6), X(0x2115a54234212184), X(0x9c94d625089c9c4a), X(0x1ef0663cee1e1e78), X(0x4322528661434311), X(0xc776fc93b1c7c73b), X(0xfcb32be54ffcfcd7), X(0x0420140824040410),
            X(0x51b208a2e3515159), X(0x99bcc72f2599995e), X(0x6d4fc4da226d6da9), X(0x0d68391a650d0d34), X(0xfa8335e979fafacf), X(0xdfb684a369dfdf5b), X(0x7ed79bfca97e7ee5), X(0x243db44819242490),
            X(0x3bc5d776fe3b3bec), X(0xab313d4b9aabab96), X(0xce3ed181f0cece1f), X(0x1188552299111144), X(0x8f0c8903838f8f06), X(0x4e4a6b9c044e4e25), X(0xb7d1517366b7b7e6), X(0xeb0b60cbe0ebeb8b),
            X(0x3cfdcc78c13c3cf0), X(0x817cbf1ffd81813e), X(0x94d4fe354094946a), X(0xf7eb0cf31cf7f7fb), X(0xb9a1676f18b9b9de), X(0x13985f268b13134c), X(0x2c7d9c58512c2cb0), X(0xd3d6b8bb05d3d36b),
            X(0xe76b5cd38ce7e7bb), X(0x6e57cbdc396e6ea5), X(0xc46ef395aac4c437), X(0x03180f061b03030c), X(0x568a13acdc565645), X(0x441a49885e44440d), X(0x7fdf9efea07f7fe1), X(0xa921374f88a9a99e),
            X(0x2a4d8254672a2aa8), X(0xbbb16d6b0abbbbd6), X(0xc146e29f87c1c123), X(0x53a202a6f1535351), X(0xdcae8ba572dcdc57), X(0x0b582716530b0b2c), X(0x9d9cd327019d9d4e), X(0x6c47c1d82b6c6cad),
            X(0x3195f562a43131c4), X(0x7487b9e8f37474cd), X(0xf6e309f115f6f6ff), X(0x460a438c4c464605), X(0xac092645a5acac8a), X(0x893c970fb589891e), X(0x14a04428b4141450), X(0xe15b42dfbae1e1a3),
            X(0x16b04e2ca6161658), X(0x3acdd274f73a3ae8), X(0x696fd0d2066969b9), X(0x09482d1241090924), X(0x70a7ade0d77070dd), X(0xb6d954716fb6b6e2), X(0xd0ceb7bd1ed0d067), X(0xed3b7ec7d6eded93),
            X(0xcc2edb85e2cccc17), X(0x422a578468424215), X(0x98b4c22d2c98985a), X(0xa4490e55eda4a4aa), X(0x285d8850752828a0), X(0x5cda31b8865c5c6d), X(0xf8933fed6bf8f8c7), X(0x8644a411c2868622)
        };

        static const uint64 C6[] =
        {   X(0x6018c07830d81818), X(0x8c2305af46262323), X(0x3fc67ef991b8c6c6), X(0x87e8136fcdfbe8e8), X(0x26874ca113cb8787), X(0xdab8a9626d11b8b8), X(0x0401080502090101), X(0x214f426e9e0d4f4f),
            X(0xd836adee6c9b3636), X(0xa2a6590451ffa6a6), X(0x6fd2debdb90cd2d2), X(0xf3f5fb06f70ef5f5), X(0xf979ef80f2967979), X(0xa16f5fcede306f6f), X(0x7e91fcef3f6d9191), X(0x5552aa07a4f85252),
            X(0x9d6027fdc0476060), X(0xcabc89766535bcbc), X(0x569baccd2b379b9b), X(0x028e048c018a8e8e), X(0xb6a371155bd2a3a3), X(0x300c603c186c0c0c), X(0xf17bff8af6847b7b), X(0xd435b5e16a803535),
            X(0x741de8693af51d1d), X(0xa7e05347ddb3e0e0), X(0x7bd7f6acb321d7d7), X(0x2fc25eed999cc2c2), X(0xb82e6d965c432e2e), X(0x314b627a96294b4b), X(0xdffea321e15dfefe), X(0x41578216aed55757),
            X(0x5415a8412abd1515), X(0xc1779fb6eee87777), X(0xdc37a5eb6e923737), X(0xb3e57b56d79ee5e5), X(0x469f8cd923139f9f), X(0xe7f0d317fd23f0f0), X(0x354a6a7f94204a4a), X(0x4fda9e95a944dada),
            X(0x7d58fa25b0a25858), X(0x03c906ca8fcfc9c9), X(0xa429558d527c2929), X(0x280a5022145a0a0a), X(0xfeb1e14f7f50b1b1), X(0xbaa0691a5dc9a0a0), X(0xb16b7fdad6146b6b), X(0x2e855cab17d98585),
            X(0xcebd8173673cbdbd), X(0x695dd234ba8f5d5d), X(0x4010805020901010), X(0xf7f4f303f507f4f4), X(0x0bcb16c08bddcbcb), X(0xf83eedc67cd33e3e), X(0x140528110a2d0505), X(0x81671fe6ce786767),
            X(0xb7e47353d597e4e4), X(0x9c2725bb4e022727), X(0x1941325882734141), X(0x168b2c9d0ba78b8b), X(0xa6a7510153f6a7a7), X(0xe97dcf94fab27d7d), X(0x6e95dcfb37499595), X(0x47d88e9fad56d8d8),
            X(0xcbfb8b30eb70fbfb), X(0x9fee2371c1cdeeee), X(0xed7cc791f8bb7c7c), X(0x856617e3cc716666), X(0x53dda68ea77bdddd), X(0x5c17b84b2eaf1717), X(0x014702468e454747), X(0x429e84dc211a9e9e),
            X(0x0fca1ec589d4caca), X(0xb42d75995a582d2d), X(0xc6bf9179632ebfbf), X(0x1c07381b0e3f0707), X(0x8ead012347acadad), X(0x755aea2fb4b05a5a), X(0x36836cb51bef8383), X(0xcc3385ff66b63333),
            X(0x91633ff2c65c6363), X(0x0802100a04120202), X(0x92aa39384993aaaa), X(0xd971afa8e2de7171), X(0x07c80ecf8dc6c8c8), X(0x6419c87d32d11919), X(0x39497270923b4949), X(0x43d9869aaf5fd9d9),
            X(0xeff2c31df931f2f2), X(0xabe34b48dba8e3e3), X(0x715be22ab6b95b5b), X(0x1a8834920dbc8888), X(0x529aa4c8293e9a9a), X(0x98262dbe4c0b2626), X(0xc8328dfa64bf3232), X(0xfab0e94a7d59b0b0),
            X(0x83e91b6acff2e9e9), X(0x3c0f78331e770f0f), X(0x73d5e6a6b733d5d5), X(0x3a8074ba1df48080), X(0xc2be997c6127bebe), X(0x13cd26de87ebcdcd), X(0xd034bde468893434), X(0x3d487a7590324848),
            X(0xdbffab24e354ffff), X(0xf57af78ff48d7a7a), X(0x7a90f4ea3d649090), X(0x615fc23ebe9d5f5f), X(0x80201da0403d2020), X(0xbd6867d5d00f6868), X(0x681ad07234ca1a1a), X(0x82ae192c41b7aeae),
            X(0xeab4c95e757db4b4), X(0x4d549a19a8ce5454), X(0x7693ece53b7f9393), X(0x88220daa442f2222), X(0x8d6407e9c8636464), X(0xe3f1db12ff2af1f1), X(0xd173bfa2e6cc7373), X(0x4812905a24821212),
            X(0x1d403a5d807a4040), X(0x2008402810480808), X(0x2bc356e89b95c3c3), X(0x97ec337bc5dfecec), X(0x4bdb9690ab4ddbdb), X(0xbea1611f5fc0a1a1), X(0x0e8d1c8307918d8d), X(0xf43df5c97ac83d3d),
            X(0x6697ccf1335b9797), X(0x0000000000000000), X(0x1bcf36d483f9cfcf), X(0xac2b4587566e2b2b), X(0xc57697b3ece17676), X(0x328264b019e68282), X(0x7fd6fea9b128d6d6), X(0x6c1bd87736c31b1b),
            X(0xeeb5c15b7774b5b5), X(0x86af112943beafaf), X(0xb56a77dfd41d6a6a), X(0x5d50ba0da0ea5050), X(0x0945124c8a574545), X(0xebf3cb18fb38f3f3), X(0xc0309df060ad3030), X(0x9bef2b74c3c4efef),
            X(0xfc3fe5c37eda3f3f), X(0x4955921caac75555), X(0xb2a2791059dba2a2), X(0x8fea0365c9e9eaea), X(0x89650fecca6a6565), X(0xd2bab9686903baba), X(0xbc2f65935e4a2f2f), X(0x27c04ee79d8ec0c0),
            X(0x5fdebe81a160dede), X(0x701ce06c38fc1c1c), X(0xd3fdbb2ee746fdfd), X(0x294d52649a1f4d4d), X(0x7292e4e039769292), X(0xc9758fbceafa7575), X(0x1806301e0c360606), X(0x128a249809ae8a8a),
            X(0xf2b2f940794bb2b2), X(0xbfe66359d185e6e6), X(0x380e70361c7e0e0e), X(0x7c1ff8633ee71f1f), X(0x956237f7c4556262), X(0x77d4eea3b53ad4d4), X(0x9aa829324d81a8a8), X(0x6296c4f431529696),
            X(0xc3f99b3aef62f9f9), X(0x33c566f697a3c5c5), X(0x942535b14a102525), X(0x7959f220b2ab5959), X(0x2a8454ae15d08484), X(0xd572b7a7e4c57272), X(0xe439d5dd72ec3939), X(0x2d4c5a6198164c4c),
            X(0x655eca3bbc945e5e), X(0xfd78e785f09f7878), X(0xe038ddd870e53838), X(0x0a8c148605988c8c), X(0x63d1c6b2bf17d1d1), X(0xaea5410b57e4a5a5), X(0xafe2434dd9a1e2e2), X(0x99612ff8c24e6161),
            X(0xf6b3f1457b42b3b3), X(0x842115a542342121), X(0x4a9c94d625089c9c), X(0x781ef0663cee1e1e), X(0x1143225286614343), X(0x3bc776fc93b1c7c7), X(0xd7fcb32be54ffcfc), X(0x1004201408240404),
            X(0x5951b208a2e35151), X(0x5e99bcc72f259999), X(0xa96d4fc4da226d6d), X(0x340d68391a650d0d), X(0xcffa8335e979fafa), X(0x5bdfb684a369dfdf), X(0xe57ed79bfca97e7e), X(0x90243db448192424),
            X(0xec3bc5d776fe3b3b), X(0x96ab313d4b9aabab), X(0x1fce3ed181f0cece), X(0x4411885522991111), X(0x068f0c8903838f8f), X(0x254e4a6b9c044e4e), X(0xe6b7d1517366b7b7), X(0x8beb0b60cbe0ebeb),
            X(0xf03cfdcc78c13c3c), X(0x3e817cbf1ffd8181), X(0x6a94d4fe35409494), X(0xfbf7eb0cf31cf7f7), X(0xdeb9a1676f18b9b9), X(0x4c13985f268b1313), X(0xb02c7d9c58512c2c), X(0x6bd3d6b8bb05d3d3),
            X(0xbbe76b5cd38ce7e7), X(0xa56e57cbdc396e6e), X(0x37c46ef395aac4c4), X(0x0c03180f061b0303), X(0x45568a13acdc5656), X(0x0d441a49885e4444), X(0xe17fdf9efea07f7f), X(0x9ea921374f88a9a9),
            X(0xa82a4d8254672a2a), X(0xd6bbb16d6b0abbbb), X(0x23c146e29f87c1c1), X(0x5153a202a6f15353), X(0x57dcae8ba572dcdc), X(0x2c0b582716530b0b), X(0x4e9d9cd327019d9d), X(0xad6c47c1d82b6c6c),
            X(0xc43195f562a43131), X(0xcd7487b9e8f37474), X(0xfff6e309f115f6f6), X(0x05460a438c4c4646), X(0x8aac092645a5acac), X(0x1e893c970fb58989), X(0x5014a04428b41414), X(0xa3e15b42dfbae1e1),
            X(0x5816b04e2ca61616), X(0xe83acdd274f73a3a), X(0xb9696fd0d2066969), X(0x2409482d12410909), X(0xdd70a7ade0d77070), X(0xe2b6d954716fb6b6), X(0x67d0ceb7bd1ed0d0), X(0x93ed3b7ec7d6eded),
            X(0x17cc2edb85e2cccc), X(0x15422a5784684242), X(0x5a98b4c22d2c9898), X(0xaaa4490e55eda4a4), X(0xa0285d8850752828), X(0x6d5cda31b8865c5c), X(0xc7f8933fed6bf8f8), X(0x228644a411c28686)
        };

        static const uint64 C7[] =
        {   X(0x186018c07830d818), X(0x238c2305af462623), X(0xc63fc67ef991b8c6), X(0xe887e8136fcdfbe8), X(0x8726874ca113cb87), X(0xb8dab8a9626d11b8), X(0x0104010805020901), X(0x4f214f426e9e0d4f),
            X(0x36d836adee6c9b36), X(0xa6a2a6590451ffa6), X(0xd26fd2debdb90cd2), X(0xf5f3f5fb06f70ef5), X(0x79f979ef80f29679), X(0x6fa16f5fcede306f), X(0x917e91fcef3f6d91), X(0x525552aa07a4f852),
            X(0x609d6027fdc04760), X(0xbccabc89766535bc), X(0x9b569baccd2b379b), X(0x8e028e048c018a8e), X(0xa3b6a371155bd2a3), X(0x0c300c603c186c0c), X(0x7bf17bff8af6847b), X(0x35d435b5e16a8035),
            X(0x1d741de8693af51d), X(0xe0a7e05347ddb3e0), X(0xd77bd7f6acb321d7), X(0xc22fc25eed999cc2), X(0x2eb82e6d965c432e), X(0x4b314b627a96294b), X(0xfedffea321e15dfe), X(0x5741578216aed557),
            X(0x155415a8412abd15), X(0x77c1779fb6eee877), X(0x37dc37a5eb6e9237), X(0xe5b3e57b56d79ee5), X(0x9f469f8cd923139f), X(0xf0e7f0d317fd23f0), X(0x4a354a6a7f94204a), X(0xda4fda9e95a944da),
            X(0x587d58fa25b0a258), X(0xc903c906ca8fcfc9), X(0x29a429558d527c29), X(0x0a280a5022145a0a), X(0xb1feb1e14f7f50b1), X(0xa0baa0691a5dc9a0), X(0x6bb16b7fdad6146b), X(0x852e855cab17d985),
            X(0xbdcebd8173673cbd), X(0x5d695dd234ba8f5d), X(0x1040108050209010), X(0xf4f7f4f303f507f4), X(0xcb0bcb16c08bddcb), X(0x3ef83eedc67cd33e), X(0x05140528110a2d05), X(0x6781671fe6ce7867),
            X(0xe4b7e47353d597e4), X(0x279c2725bb4e0227), X(0x4119413258827341), X(0x8b168b2c9d0ba78b), X(0xa7a6a7510153f6a7), X(0x7de97dcf94fab27d), X(0x956e95dcfb374995), X(0xd847d88e9fad56d8),
            X(0xfbcbfb8b30eb70fb), X(0xee9fee2371c1cdee), X(0x7ced7cc791f8bb7c), X(0x66856617e3cc7166), X(0xdd53dda68ea77bdd), X(0x175c17b84b2eaf17), X(0x47014702468e4547), X(0x9e429e84dc211a9e),
            X(0xca0fca1ec589d4ca), X(0x2db42d75995a582d), X(0xbfc6bf9179632ebf), X(0x071c07381b0e3f07), X(0xad8ead012347acad), X(0x5a755aea2fb4b05a), X(0x8336836cb51bef83), X(0x33cc3385ff66b633),
            X(0x6391633ff2c65c63), X(0x020802100a041202), X(0xaa92aa39384993aa), X(0x71d971afa8e2de71), X(0xc807c80ecf8dc6c8), X(0x196419c87d32d119), X(0x4939497270923b49), X(0xd943d9869aaf5fd9),
            X(0xf2eff2c31df931f2), X(0xe3abe34b48dba8e3), X(0x5b715be22ab6b95b), X(0x881a8834920dbc88), X(0x9a529aa4c8293e9a), X(0x2698262dbe4c0b26), X(0x32c8328dfa64bf32), X(0xb0fab0e94a7d59b0),
            X(0xe983e91b6acff2e9), X(0x0f3c0f78331e770f), X(0xd573d5e6a6b733d5), X(0x803a8074ba1df480), X(0xbec2be997c6127be), X(0xcd13cd26de87ebcd), X(0x34d034bde4688934), X(0x483d487a75903248),
            X(0xffdbffab24e354ff), X(0x7af57af78ff48d7a), X(0x907a90f4ea3d6490), X(0x5f615fc23ebe9d5f), X(0x2080201da0403d20), X(0x68bd6867d5d00f68), X(0x1a681ad07234ca1a), X(0xae82ae192c41b7ae),
            X(0xb4eab4c95e757db4), X(0x544d549a19a8ce54), X(0x937693ece53b7f93), X(0x2288220daa442f22), X(0x648d6407e9c86364), X(0xf1e3f1db12ff2af1), X(0x73d173bfa2e6cc73), X(0x124812905a248212),
            X(0x401d403a5d807a40), X(0x0820084028104808), X(0xc32bc356e89b95c3), X(0xec97ec337bc5dfec), X(0xdb4bdb9690ab4ddb), X(0xa1bea1611f5fc0a1), X(0x8d0e8d1c8307918d), X(0x3df43df5c97ac83d),
            X(0x976697ccf1335b97), X(0x0000000000000000), X(0xcf1bcf36d483f9cf), X(0x2bac2b4587566e2b), X(0x76c57697b3ece176), X(0x82328264b019e682), X(0xd67fd6fea9b128d6), X(0x1b6c1bd87736c31b),
            X(0xb5eeb5c15b7774b5), X(0xaf86af112943beaf), X(0x6ab56a77dfd41d6a), X(0x505d50ba0da0ea50), X(0x450945124c8a5745), X(0xf3ebf3cb18fb38f3), X(0x30c0309df060ad30), X(0xef9bef2b74c3c4ef),
            X(0x3ffc3fe5c37eda3f), X(0x554955921caac755), X(0xa2b2a2791059dba2), X(0xea8fea0365c9e9ea), X(0x6589650fecca6a65), X(0xbad2bab9686903ba), X(0x2fbc2f65935e4a2f), X(0xc027c04ee79d8ec0),
            X(0xde5fdebe81a160de), X(0x1c701ce06c38fc1c), X(0xfdd3fdbb2ee746fd), X(0x4d294d52649a1f4d), X(0x927292e4e0397692), X(0x75c9758fbceafa75), X(0x061806301e0c3606), X(0x8a128a249809ae8a),
            X(0xb2f2b2f940794bb2), X(0xe6bfe66359d185e6), X(0x0e380e70361c7e0e), X(0x1f7c1ff8633ee71f), X(0x62956237f7c45562), X(0xd477d4eea3b53ad4), X(0xa89aa829324d81a8), X(0x966296c4f4315296),
            X(0xf9c3f99b3aef62f9), X(0xc533c566f697a3c5), X(0x25942535b14a1025), X(0x597959f220b2ab59), X(0x842a8454ae15d084), X(0x72d572b7a7e4c572), X(0x39e439d5dd72ec39), X(0x4c2d4c5a6198164c),
            X(0x5e655eca3bbc945e), X(0x78fd78e785f09f78), X(0x38e038ddd870e538), X(0x8c0a8c148605988c), X(0xd163d1c6b2bf17d1), X(0xa5aea5410b57e4a5), X(0xe2afe2434dd9a1e2), X(0x6199612ff8c24e61),
            X(0xb3f6b3f1457b42b3), X(0x21842115a5423421), X(0x9c4a9c94d625089c), X(0x1e781ef0663cee1e), X(0x4311432252866143), X(0xc73bc776fc93b1c7), X(0xfcd7fcb32be54ffc), X(0x0410042014082404),
            X(0x515951b208a2e351), X(0x995e99bcc72f2599), X(0x6da96d4fc4da226d), X(0x0d340d68391a650d), X(0xfacffa8335e979fa), X(0xdf5bdfb684a369df), X(0x7ee57ed79bfca97e), X(0x2490243db4481924),
            X(0x3bec3bc5d776fe3b), X(0xab96ab313d4b9aab), X(0xce1fce3ed181f0ce), X(0x1144118855229911), X(0x8f068f0c8903838f), X(0x4e254e4a6b9c044e), X(0xb7e6b7d1517366b7), X(0xeb8beb0b60cbe0eb),
            X(0x3cf03cfdcc78c13c), X(0x813e817cbf1ffd81), X(0x946a94d4fe354094), X(0xf7fbf7eb0cf31cf7), X(0xb9deb9a1676f18b9), X(0x134c13985f268b13), X(0x2cb02c7d9c58512c), X(0xd36bd3d6b8bb05d3),
            X(0xe7bbe76b5cd38ce7), X(0x6ea56e57cbdc396e), X(0xc437c46ef395aac4), X(0x030c03180f061b03), X(0x5645568a13acdc56), X(0x440d441a49885e44), X(0x7fe17fdf9efea07f), X(0xa99ea921374f88a9),
            X(0x2aa82a4d8254672a), X(0xbbd6bbb16d6b0abb), X(0xc123c146e29f87c1), X(0x535153a202a6f153), X(0xdc57dcae8ba572dc), X(0x0b2c0b582716530b), X(0x9d4e9d9cd327019d), X(0x6cad6c47c1d82b6c),
            X(0x31c43195f562a431), X(0x74cd7487b9e8f374), X(0xf6fff6e309f115f6), X(0x4605460a438c4c46), X(0xac8aac092645a5ac), X(0x891e893c970fb589), X(0x145014a04428b414), X(0xe1a3e15b42dfbae1),
            X(0x165816b04e2ca616), X(0x3ae83acdd274f73a), X(0x69b9696fd0d20669), X(0x092409482d124109), X(0x70dd70a7ade0d770), X(0xb6e2b6d954716fb6), X(0xd067d0ceb7bd1ed0), X(0xed93ed3b7ec7d6ed),
            X(0xcc17cc2edb85e2cc), X(0x4215422a57846842), X(0x985a98b4c22d2c98), X(0xa4aaa4490e55eda4), X(0x28a0285d88507528), X(0x5c6d5cda31b8865c), X(0xf8c7f8933fed6bf8), X(0x86228644a411c286)
        };

        #undef X

        uint64 block[8];
        const uint8* b = buffer;

        for (int i = 0; i < 8; ++i, b += 8)
            block[i] = (((uint64) b[0]       ) << 56)
                     ^ (((uint64) b[1] & 0xff) << 48)
                     ^ (((uint64) b[2] & 0xff) << 40)
                     ^ (((uint64) b[3] & 0xff) << 32)
                     ^ (((uint64) b[4] & 0xff) << 24)
                     ^ (((uint64) b[5] & 0xff) << 16)
                     ^ (((uint64) b[6] & 0xff) <<  8)
                     ^ (((uint64) b[7] & 0xff));

        uint64 state[8], K[8];

        state[0] = block[0] ^ (K[0] = hash[0]);
        state[1] = block[1] ^ (K[1] = hash[1]);
        state[2] = block[2] ^ (K[2] = hash[2]);
        state[3] = block[3] ^ (K[3] = hash[3]);
        state[4] = block[4] ^ (K[4] = hash[4]);
        state[5] = block[5] ^ (K[5] = hash[5]);
        state[6] = block[6] ^ (K[6] = hash[6]);
        state[7] = block[7] ^ (K[7] = hash[7]);

        for (int i = 0; i < 10; ++i)
        {
            uint64 L[8];

            L[0] = C0[(int) (K[0] >> 56)] ^ C1[(int) (K[7] >> 48) & 0xff] ^ C2[(int) (K[6] >> 40) & 0xff] ^ C3[(int) (K[5] >> 32) & 0xff] ^ C4[(int) (K[4] >> 24) & 0xff] ^ C5[(int) (K[3] >> 16) & 0xff] ^ C6[(int) (K[2] >>  8) & 0xff] ^ C7[(int) (K[1]) & 0xff] ^ rc[i];
            L[1] = C0[(int) (K[1] >> 56)] ^ C1[(int) (K[0] >> 48) & 0xff] ^ C2[(int) (K[7] >> 40) & 0xff] ^ C3[(int) (K[6] >> 32) & 0xff] ^ C4[(int) (K[5] >> 24) & 0xff] ^ C5[(int) (K[4] >> 16) & 0xff] ^ C6[(int) (K[3] >>  8) & 0xff] ^ C7[(int) (K[2]) & 0xff];
            L[2] = C0[(int) (K[2] >> 56)] ^ C1[(int) (K[1] >> 48) & 0xff] ^ C2[(int) (K[0] >> 40) & 0xff] ^ C3[(int) (K[7] >> 32) & 0xff] ^ C4[(int) (K[6] >> 24) & 0xff] ^ C5[(int) (K[5] >> 16) & 0xff] ^ C6[(int) (K[4] >>  8) & 0xff] ^ C7[(int) (K[3]) & 0xff];
            L[3] = C0[(int) (K[3] >> 56)] ^ C1[(int) (K[2] >> 48) & 0xff] ^ C2[(int) (K[1] >> 40) & 0xff] ^ C3[(int) (K[0] >> 32) & 0xff] ^ C4[(int) (K[7] >> 24) & 0xff] ^ C5[(int) (K[6] >> 16) & 0xff] ^ C6[(int) (K[5] >>  8) & 0xff] ^ C7[(int) (K[4]) & 0xff];
            L[4] = C0[(int) (K[4] >> 56)] ^ C1[(int) (K[3] >> 48) & 0xff] ^ C2[(int) (K[2] >> 40) & 0xff] ^ C3[(int) (K[1] >> 32) & 0xff] ^ C4[(int) (K[0] >> 24) & 0xff] ^ C5[(int) (K[7] >> 16) & 0xff] ^ C6[(int) (K[6] >>  8) & 0xff] ^ C7[(int) (K[5]) & 0xff];
            L[5] = C0[(int) (K[5] >> 56)] ^ C1[(int) (K[4] >> 48) & 0xff] ^ C2[(int) (K[3] >> 40) & 0xff] ^ C3[(int) (K[2] >> 32) & 0xff] ^ C4[(int) (K[1] >> 24) & 0xff] ^ C5[(int) (K[0] >> 16) & 0xff] ^ C6[(int) (K[7] >>  8) & 0xff] ^ C7[(int) (K[6]) & 0xff];
            L[6] = C0[(int) (K[6] >> 56)] ^ C1[(int) (K[5] >> 48) & 0xff] ^ C2[(int) (K[4] >> 40) & 0xff] ^ C3[(int) (K[3] >> 32) & 0xff] ^ C4[(int) (K[2] >> 24) & 0xff] ^ C5[(int) (K[1] >> 16) & 0xff] ^ C6[(int) (K[0] >>  8) & 0xff] ^ C7[(int) (K[7]) & 0xff];
            L[7] = C0[(int) (K[7] >> 56)] ^ C1[(int) (K[6] >> 48) & 0xff] ^ C2[(int) (K[5] >> 40) & 0xff] ^ C3[(int) (K[4] >> 32) & 0xff] ^ C4[(int) (K[3] >> 24) & 0xff] ^ C5[(int) (K[2] >> 16) & 0xff] ^ C6[(int) (K[1] >>  8) & 0xff] ^ C7[(int) (K[0]) & 0xff];

            K[0] = L[0];
            K[1] = L[1];
            K[2] = L[2];
            K[3] = L[3];
            K[4] = L[4];
            K[5] = L[5];
            K[6] = L[6];
            K[7] = L[7];

            // apply the i-th round transformation
            L[0] = C0[(int) (state[0] >> 56)] ^ C1[(int) (state[7] >> 48) & 0xff] ^ C2[(int) (state[6] >> 40) & 0xff] ^ C3[(int) (state[5] >> 32) & 0xff] ^ C4[(int) (state[4] >> 24) & 0xff] ^ C5[(int) (state[3] >> 16) & 0xff] ^ C6[(int) (state[2] >>  8) & 0xff] ^ C7[(int) (state[1]) & 0xff] ^ K[0];
            L[1] = C0[(int) (state[1] >> 56)] ^ C1[(int) (state[0] >> 48) & 0xff] ^ C2[(int) (state[7] >> 40) & 0xff] ^ C3[(int) (state[6] >> 32) & 0xff] ^ C4[(int) (state[5] >> 24) & 0xff] ^ C5[(int) (state[4] >> 16) & 0xff] ^ C6[(int) (state[3] >>  8) & 0xff] ^ C7[(int) (state[2]) & 0xff] ^ K[1];
            L[2] = C0[(int) (state[2] >> 56)] ^ C1[(int) (state[1] >> 48) & 0xff] ^ C2[(int) (state[0] >> 40) & 0xff] ^ C3[(int) (state[7] >> 32) & 0xff] ^ C4[(int) (state[6] >> 24) & 0xff] ^ C5[(int) (state[5] >> 16) & 0xff] ^ C6[(int) (state[4] >>  8) & 0xff] ^ C7[(int) (state[3]) & 0xff] ^ K[2];
            L[3] = C0[(int) (state[3] >> 56)] ^ C1[(int) (state[2] >> 48) & 0xff] ^ C2[(int) (state[1] >> 40) & 0xff] ^ C3[(int) (state[0] >> 32) & 0xff] ^ C4[(int) (state[7] >> 24) & 0xff] ^ C5[(int) (state[6] >> 16) & 0xff] ^ C6[(int) (state[5] >>  8) & 0xff] ^ C7[(int) (state[4]) & 0xff] ^ K[3];
            L[4] = C0[(int) (state[4] >> 56)] ^ C1[(int) (state[3] >> 48) & 0xff] ^ C2[(int) (state[2] >> 40) & 0xff] ^ C3[(int) (state[1] >> 32) & 0xff] ^ C4[(int) (state[0] >> 24) & 0xff] ^ C5[(int) (state[7] >> 16) & 0xff] ^ C6[(int) (state[6] >>  8) & 0xff] ^ C7[(int) (state[5]) & 0xff] ^ K[4];
            L[5] = C0[(int) (state[5] >> 56)] ^ C1[(int) (state[4] >> 48) & 0xff] ^ C2[(int) (state[3] >> 40) & 0xff] ^ C3[(int) (state[2] >> 32) & 0xff] ^ C4[(int) (state[1] >> 24) & 0xff] ^ C5[(int) (state[0] >> 16) & 0xff] ^ C6[(int) (state[7] >>  8) & 0xff] ^ C7[(int) (state[6]) & 0xff] ^ K[5];
            L[6] = C0[(int) (state[6] >> 56)] ^ C1[(int) (state[5] >> 48) & 0xff] ^ C2[(int) (state[4] >> 40) & 0xff] ^ C3[(int) (state[3] >> 32) & 0xff] ^ C4[(int) (state[2] >> 24) & 0xff] ^ C5[(int) (state[1] >> 16) & 0xff] ^ C6[(int) (state[0] >>  8) & 0xff] ^ C7[(int) (state[7]) & 0xff] ^ K[6];
            L[7] = C0[(int) (state[7] >> 56)] ^ C1[(int) (state[6] >> 48) & 0xff] ^ C2[(int) (state[5] >> 40) & 0xff] ^ C3[(int) (state[4] >> 32) & 0xff] ^ C4[(int) (state[3] >> 24) & 0xff] ^ C5[(int) (state[2] >> 16) & 0xff] ^ C6[(int) (state[1] >>  8) & 0xff] ^ C7[(int) (state[0]) & 0xff] ^ K[7];

            state[0] = L[0];
            state[1] = L[1];
            state[2] = L[2];
            state[3] = L[3];
            state[4] = L[4];
            state[5] = L[5];
            state[6] = L[6];
            state[7] = L[7];
        }

        // apply the Miyaguchi-Preneel compression function
        hash[0] ^= state[0] ^ block[0];
        hash[1] ^= state[1] ^ block[1];
        hash[2] ^= state[2] ^ block[2];
        hash[3] ^= state[3] ^ block[3];
        hash[4] ^= state[4] ^ block[4];
        hash[5] ^= state[5] ^ block[5];
        hash[6] ^= state[6] ^ block[6];
        hash[7] ^= state[7] ^ block[7];
    }

private:
    JUCE_DECLARE_NON_COPYABLE (WhirlpoolProcessor)
};


//==============================================================================
Whirlpool::Whirlpool() noexcept
{
    zerostruct (result);
}

Whirlpool::~Whirlpool() noexcept {}

Whirlpool::Whirlpool (const Whirlpool& other) noexcept
{
    memcpy (result, other.result, sizeof (result));
}

Whirlpool& Whirlpool::operator= (const Whirlpool& other) noexcept
{
    memcpy (result, other.result, sizeof (result));
    return *this;
}

Whirlpool::Whirlpool (const MemoryBlock& data)
{
    process (data.getData(), data.getSize());
}

Whirlpool::Whirlpool (const void* const data, const size_t numBytes)
{
    process (data, numBytes);
}

Whirlpool::Whirlpool (InputStream& input, const int64 numBytesToRead)
{
    WhirlpoolProcessor processor;
    processor.processStream (input, numBytesToRead, result);
}

Whirlpool::Whirlpool (const File& file)
{
    FileInputStream fin (file);

    if (fin.getStatus().wasOk())
    {
        WhirlpoolProcessor processor;
        processor.processStream (fin, -1, result);
    }
    else
    {
        zerostruct (result);
    }
}

Whirlpool::Whirlpool (CharPointer_UTF8 utf8) noexcept
{
    jassert (utf8.getAddress() != nullptr);
    process (utf8.getAddress(), utf8.sizeInBytes() - 1);
}

void Whirlpool::process (const void* const data, size_t numBytes)
{
    MemoryInputStream m (data, numBytes, false);
    WhirlpoolProcessor processor;
    processor.processStream (m, -1, result);
}

MemoryBlock Whirlpool::getRawData() const
{
    return MemoryBlock (result, sizeof (result));
}

String Whirlpool::toHexString() const
{
    return String::toHexString (result, sizeof (result), 0);
}

bool Whirlpool::operator== (const Whirlpool& other) const noexcept  { return memcmp (result, other.result, sizeof (result)) == 0; }
bool Whirlpool::operator!= (const Whirlpool& other) const noexcept  { return ! operator== (other); }


//==============================================================================
#if JUCE_UNIT_TESTS

class WhirlpoolTests  : public UnitTest
{
public:
    WhirlpoolTests() : UnitTest ("Whirlpool") {}

    void test (const char* input, const char* expected)
    {
        {
            Whirlpool hash (input, strlen (input));
            expectEquals (hash.toHexString(), String (expected));
        }

        {
            CharPointer_UTF8 utf8 (input);
            Whirlpool hash (utf8);
            expectEquals (hash.toHexString(), String (expected));
        }

        {
            MemoryInputStream m (input, strlen (input), false);
            Whirlpool hash (m);
            expectEquals (hash.toHexString(), String (expected));
        }
    }

    void runTest()
    {
        beginTest ("Whirlpool");

        test ("", "19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f0757ea8964e59b63d93708b138cc42a66eb3");
        test ("The quick brown fox jumps over the lazy dog",  "b97de512e91e3828b40d2b0fdce9ceb3c4a71f9bea8d88e75c4fa854df36725fd2b52eb6544edcacd6f8beddfea403cb55ae31f03ad62a5ef54e42ee82c3fb35");
        test ("The quick brown fox jumps over the lazy dog.", "87a7ff096082e3ffeb86db10feb91c5af36c2c71bc426fe310ce662e0338223e217def0eab0b02b80eecf875657802bc5965e48f5c0a05467756f0d3f396faba");
    }
};

static WhirlpoolTests whirlpoolUnitTests;

#endif
