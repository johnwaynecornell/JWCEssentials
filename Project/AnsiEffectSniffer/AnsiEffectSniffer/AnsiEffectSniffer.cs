using System;
using System.Text;
using JWCEssentials.net;

namespace JWCEssentials
{
    public enum Brightness
    {
        Normal,
        Bright
    }

    public delegate void FgDelegate(string color, Brightness brightness);
    public delegate void BgDelegate(string color, Brightness brightness);
    public delegate void CharDelegate(char c);

    public delegate void SwitchDelegate(string mode, bool enable);
    
    public class AnsiEffectSniffer
    {
        public FgDelegate? Fg { get; set; }
        public BgDelegate? Bg { get; set; }
        public Action? Reset { get; set; }
        public CharDelegate? Char { get; set; }
        public SwitchDelegate? Switch { get; set; }

        private readonly Decoder _decoder = Encoding.UTF8.GetDecoder();
        private readonly char[] _charBuffer = new char[1];
        private readonly byte[] _byteBuffer = new byte[1];

        private ParserState _state = ParserState.Normal;
        private readonly StringBuilder _csiBuffer = new StringBuilder();

        private enum ParserState
        {
            Normal,
            Escape,
            Csi
        }

        private const int MaxCsiBufferLength = 128;

        public void Process(ReadOnlySpan<byte> bytes)
        {
            foreach (var b in bytes)
            {
                ProcessByte(b);
            }
        }

        public void ProcessByte(byte b)
        {
            switch (_state)
            {
                case ParserState.Normal:
                    if (b == 0x1B) // ESC
                    {
                        _state = ParserState.Escape;
                    }
                    else
                    {
                        DecodeAndFireChar(b);
                    }
                    break;

                case ParserState.Escape:
                    if (b == '[') // CSI
                    {
                        _csiBuffer.Clear();
                        _state = ParserState.Csi;
                    }
                    else
                    {
                        // Some other escape sequence we don't handle yet
                        _state = ParserState.Normal;
                        DecodeAndFireChar(0x1B);
                        ProcessByte(b); // Re-process as normal or new ESC
                    }
                    break;

                case ParserState.Csi:
                    if (b >= 0x40 && b <= 0x7E) // Final byte of CSI
                    {
                        HandleCsi(b, _csiBuffer.ToString());
                        _state = ParserState.Normal;
                    }
                    else
                    {
                        if (_csiBuffer.Length < MaxCsiBufferLength)
                        {
                            _csiBuffer.Append((char)b);
                        }
                        else
                        {
                            // Too long, abort CSI
                            _state = ParserState.Normal;
                            DecodeAndFireChar(0x1B);
                            DecodeAndFireChar((byte)'[');
                            foreach (var cb in _csiBuffer.ToString()) DecodeAndFireChar((byte)cb);
                            _csiBuffer.Clear();
                            ProcessByte(b);
                        }
                    }
                    break;
            }
        }

        private void DecodeAndFireChar(byte b)
        {
            _byteBuffer[0] = b;
            int charsUsed = _decoder.GetChars(_byteBuffer, 0, 1, _charBuffer, 0, false);
            if (charsUsed > 0)
            {
                for (int i = 0; i < charsUsed; i++)
                {
                    Char?.Invoke(_charBuffer[i]);
                }
            }
        }

        private void HandleCsi(byte finalByte, string parameters)
        {
            if (finalByte == 'm') // SGR
            {
                var parts = parameters.Split(';');
                foreach (var part in parts)
                {
                    if (int.TryParse(part, out int code))
                    {
                        HandleSgrCode(code);
                    }
                    else if (string.IsNullOrEmpty(part))
                    {
                        HandleSgrCode(0); // Reset
                    }
                }
            }
        }

        private void HandleSgrCode(int code)
        {
            if (code == 0)
            {
                Reset?.Invoke();
            }
            else if (code >= 30 && code <= 37)
            {
                Fg?.Invoke(GetColorName(code - 30), Brightness.Normal);
            }
            else if (code >= 90 && code <= 97)
            {
                Fg?.Invoke(GetColorName(code - 90), Brightness.Bright);
            }
            else if (code == 39)
            {
                Fg?.Invoke("default", Brightness.Normal);
            }
            else if (code >= 40 && code <= 47)
            {
                Bg?.Invoke(GetColorName(code - 40), Brightness.Normal);
            }
            else if (code >= 100 && code <= 107)
            {
                Bg?.Invoke(GetColorName(code - 100), Brightness.Bright);
            }
            else if (code == 49)
            {
                Bg?.Invoke("default", Brightness.Normal);
            }
            else
            {
                string name = Essentials.feffect_name(code.ToString());
                if (name != null)
                {
                    bool off = name.EndsWith("_off"); 
                    
                    if (off || Essentials.feffect_code(name+"_off") != null)
                    {
                        if (off) name = name.Substring(0, name.Length - 4);
                        
                        Switch?.Invoke(name, !off);
                    }
                }
            }
            // Add more codes as needed (bold, etc.)
        }


        public static string[] colors = new[]
        {
            "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"
        };
        
        public static string[] Switches { get; } = 
        {
            "bold", "italic","underline", "blink", "reverse", "crossed", "overline"
        };
        
        private string GetColorName(int index)
        {
            return index switch
            {
                0 => "black",
                1 => "red",
                2 => "green",
                3 => "yellow",
                4 => "blue",
                5 => "magenta",
                6 => "cyan",
                7 => "white",
                _ => "unknown"
            };
        }
    }
}
