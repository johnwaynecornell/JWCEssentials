using Xunit;
using JWCEssentials;
using System.Text;
using System.Collections.Generic;

namespace AnsiEffectSniffer.Tests
{
    public class SnifferTests
    {
        [Fact]
        public void TestNormalChars()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            var result = new StringBuilder();
            sniffer.Char = (c) => result.Append(c);

            sniffer.Process(Encoding.UTF8.GetBytes("Hello World"));

            Assert.Equal("Hello World", result.ToString());
        }

        [Fact]
        public void TestFgColor()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            string? lastColor = null;
            Brightness lastBrightness = Brightness.Normal;

            sniffer.Fg = (color, brightness) =>
            {
                lastColor = color;
                lastBrightness = brightness;
            };

            // ESC [ 31 m (Red)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'3', (byte)'1', (byte)'m' });

            Assert.Equal("red", lastColor);
            Assert.Equal(Brightness.Normal, lastBrightness);

            // ESC [ 92 m (Bright Green)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'9', (byte)'2', (byte)'m' });

            Assert.Equal("green", lastColor);
            Assert.Equal(Brightness.Bright, lastBrightness);
        }

        [Fact]
        public void TestUtf8Chars()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            var result = new StringBuilder();
            sniffer.Char = (c) => result.Append(c);

            // "π" in UTF-8 is 0xCF 0x80
            sniffer.Process(new byte[] { 0xCF, 0x80 });

            Assert.Equal("π", result.ToString());
        }

        [Fact]
        public void TestFragmentedBytes()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            var result = new StringBuilder();
            sniffer.Char = (c) => result.Append(c);
            string? lastColor = null;

            sniffer.Fg = (color, brightness) => lastColor = color;

            // Send ESC [ 31 m H e l l o one byte at a time
            byte[] bytes = Encoding.UTF8.GetBytes("\x1B[31mHello");
            foreach (var b in bytes)
            {
                sniffer.Process(new byte[] { b });
            }

            Assert.Equal("red", lastColor);
            Assert.Equal("Hello", result.ToString());
        }

        [Fact]
        public void TestMultipleSgrCodes()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            var colors = new List<string>();

            sniffer.Fg = (color, brightness) => colors.Add(color);

            // ESC [ 31 ; 92 m (Red then Bright Green)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'3', (byte)'1', (byte)';', (byte)'9', (byte)'2', (byte)'m' });

            Assert.Equal(2, colors.Count);
            Assert.Equal("red", colors[0]);
            Assert.Equal("green", colors[1]);
        }

        [Fact]
        public void TestBgAndReset()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            string? lastBg = null;
            bool resetCalled = false;

            sniffer.Bg = (color, brightness) => lastBg = color;
            sniffer.Reset = () => resetCalled = true;

            // ESC [ 44 m (Blue BG)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'4', (byte)'4', (byte)'m' });
            Assert.Equal("blue", lastBg);

            // ESC [ 0 m (Reset)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'0', (byte)'m' });
            Assert.True(resetCalled);
        }

        [Fact]
        public void TestSwitchFunctionality()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            var switches = new List<(string mode, bool enable)>();

            sniffer.Switch = (mode, enable) => switches.Add((mode, enable));

            // ESC [ 1 m (Bold)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'1', (byte)'m' });
            
            // ESC [ 3 m (Italic)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'3', (byte)'m' });

            // ESC [ 22 m (Bold off)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'2', (byte)'2', (byte)'m' });

            Assert.NotEmpty(switches);
            // We assume standard ANSI codes map to these names in Essentials.feffect_name
            // 1 -> bold
            // 3 -> italic
            // 22 -> bold_off -> bold, false
            
            Assert.Contains(switches, s => s.mode == "bold" && s.enable);
            Assert.Contains(switches, s => s.mode == "italic" && s.enable);
            Assert.Contains(switches, s => s.mode == "bold" && !s.enable);
        }

        [Fact]
        public void TestFontFunctionality()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            var fonts = new List<int>();

            sniffer.Font = (number) => fonts.Add(number);

            // ESC [ 10 m (Font 0)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'1', (byte)'0', (byte)'m' });
            
            // ESC [ 11 m (Font 1)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'1', (byte)'1', (byte)'m' });

            // ESC [ 19 m (Font 9)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'1', (byte)'9', (byte)'m' });

            Assert.Equal(3, fonts.Count);
            Assert.Equal(0, fonts[0]);
            Assert.Equal(1, fonts[1]);
            Assert.Equal(9, fonts[2]);
        }

        [Fact]
        public void TestMultipleFontChanges()
        {
            var sniffer = new JWCEssentials.AnsiEffectSniffer();
            var fonts = new List<int>();

            sniffer.Font = (number) => fonts.Add(number);

            // ESC [ 11 ; 12 m (Font 1 then Font 2)
            sniffer.Process(new byte[] { 0x1B, (byte)'[', (byte)'1', (byte)'1', (byte)';', (byte)'1', (byte)'2', (byte)'m' });

            Assert.Equal(2, fonts.Count);
            Assert.Equal(1, fonts[0]);
            Assert.Equal(2, fonts[1]);
        }
    }
}
