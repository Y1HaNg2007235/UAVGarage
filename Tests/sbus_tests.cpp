#include "Libraries/Remote/SbusDecoder.h"
#include "Libraries/Remote/StickTarget.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static unsigned checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); exit(1); } } while (0)
static bool near(float a, float b) { return fabsf(a-b) < 0.002f; }
static void pack(uint8_t *bytes, const uint16_t *channels, uint8_t flags=0, uint8_t footer=0)
{
    memset(bytes,0,25); bytes[0]=15; bytes[23]=flags; bytes[24]=footer;
    for (unsigned c=0;c<16;++c) for(unsigned bit=0;bit<11;++bit)
    {
        const unsigned index=c*11+bit;
        bytes[1+index/8] |= ((channels[c]>>bit)&1U)<<(index%8);
    }
}
static bool feed(garage::SbusDecoder &decoder, const uint8_t *bytes, uint32_t now)
{
    bool got=false;
    for(unsigned i=0;i<25;++i) { got=decoder.push(bytes[i],now+i/8U)||got; }
    return got;
}
static void decoderTests()
{
    garage::SbusDecoder decoder; uint16_t channels[16]; uint8_t bytes[25];
    CHECK(!decoder.usable(0));
    // Exercise every bit position in every channel, including crossings between packed bytes.
    for(unsigned value=0;value<2048;++value)
    {
        for(unsigned c=0;c<16;++c) { channels[c]=(value+c*137U)&2047U; }
        pack(bytes,channels);
        CHECK(feed(decoder,bytes,value*14U));
        for(unsigned c=0;c<16;++c) { CHECK(decoder.frame().channels[c]==channels[c]); }
    }
    CHECK(decoder.usable(2047U*14U+3));
    CHECK(!decoder.usable(2047U*14U+104));
    pack(bytes,channels,0x0F); CHECK(feed(decoder,bytes,30000));
    CHECK(decoder.frame().frameLost && decoder.frame().failsafe);
    CHECK(decoder.frame().channel17 && decoder.frame().channel18 && !decoder.usable(30003));
    pack(bytes,channels,4); CHECK(feed(decoder,bytes,30014)); CHECK(!decoder.usable(30017));
    pack(bytes,channels,8); CHECK(feed(decoder,bytes,30028)); CHECK(!decoder.usable(30031));
    pack(bytes,channels); CHECK(feed(decoder,bytes,30042)); CHECK(decoder.usable(30045));
    decoder.discardPartial(); CHECK(!decoder.usable(30045));
    bytes[24]=0xFF; CHECK(!feed(decoder,bytes,30056));
    pack(bytes,channels,0x80); CHECK(!feed(decoder,bytes,30070));
    pack(bytes,channels); CHECK(feed(decoder,bytes,30084));
    for(unsigned footer=4;footer<=0x34;footer+=0x10)
    {
        pack(bytes,channels,0,footer); CHECK(feed(decoder,bytes,30100+footer*14));
    }
    decoder.discardPartial(); pack(bytes,channels);
    for(unsigned i=0;i<12;++i) decoder.push(bytes[i],40000);
    CHECK(feed(decoder,bytes,40020)); // Truncated frame, then a gap and full frame.
    decoder.discardPartial();
    for(unsigned i=0;i<25;++i) if(i!=7) decoder.push(bytes[i],41000+i/8);
    CHECK(feed(decoder,bytes,41003)); // Missing byte followed by an adjacent valid frame.
    CHECK(feed(decoder,bytes,0xFFFFFFFEU)); CHECK(decoder.usable(2U));
    CHECK(!decoder.usable(103U));
    puts("PASS SBUS packing, resynchronization, status flags, expiry and timestamp wrap");
}
static void targetTests()
{
    garage::StickSettings settings={172,992,1811,40,false,60,30};
    garage::StickTarget target(settings);
    CHECK(near(target.update(992,false,true,100,.02f),0) && !target.active());
    CHECK(near(target.update(992,true,true,100,.02f),100));
    CHECK(near(target.update(1811,true,true,100,.02f),101.2f));
    CHECK(near(target.demand(),1));
    CHECK(near(target.update(992,true,true,100.5f,.02f),100.5f));
    CHECK(near(target.update(992,true,true,101,.02f),100.5f)); // Hold, do not chase position.
    target.update(172,true,true,100.5f,.02f); CHECK(near(target.demand(),-1));
    CHECK(near(target.update(172,false,true,100,.02f),100));
    CHECK(near(target.update(172,false,true,99,.02f),100));
    target.reset(); target.update(992,true,true,359,.02f);
    CHECK(near(target.update(1811,true,true,359,.02f),.2f));
    target.reset(); target.update(992,true,true,1,.02f);
    CHECK(near(target.update(172,true,true,1,.02f),359.8f));
    target.reset(); target.update(992,true,true,0,.02f);
    for(unsigned i=0;i<1000;++i) target.update(1811,true,true,0,.02f);
    CHECK(near(target.target(),30)); // A blocked motor cannot accumulate a half-turn error.
    CHECK(near(target.update(1811,true,true,10,2.0f),10));
    target.update(1811,true,false,10,.02f); CHECK(!target.active());
    settings.reversed=true; garage::StickTarget reversed(settings);
    CHECK(near(reversed.update(1811,true,true,100,.02f),98.8f));
    CHECK(near(reversed.update(1030,true,true,100,.02f),100));
    settings.deadband=900; garage::StickTarget invalid(settings);
    invalid.update(1811,true,true,100,.02f); CHECK(!invalid.active());
    puts("PASS stick direction, deadband, target rate, hold, loss and angle wrap");
}
int main()
{
    decoderTests(); targetTests(); printf("ALL PASSED (%u assertions)\n",checks); return 0;
}
