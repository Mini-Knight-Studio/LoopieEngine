using System;

namespace Loopie
{
    public struct Color
    {
        private Vector4 value;

        public Color(float r, float g, float b, float a = 255f)
        {
            value = new Vector4(r, g, b, a);
        }

        public Color(Vector4 vector)
        {
            value = vector;
        }

        public float r
        {
            get => value.x;
            set => this.value.x = value;
        }

        public float g
        {
            get => value.y;
            set => this.value.y = value;
        }

        public float b
        {
            get => value.z;
            set => this.value.z = value;
        }

        public float a
        {
            get => value.w;
            set => this.value.w = value;
        }

        public Vector3 rgb
        {
            get => new Vector3(r, g, b);
            set => this.value = new Vector4(value.x, value.y, value.z, this.value.w);
        }

        public Vector4 rgba
        {
            get => value;
            set => this.value = value;
        }

        public override bool Equals(object obj)
        {
            return obj is Color c && value == c.value;
        }

        public override int GetHashCode()
        {
            return value.GetHashCode();
        }

        public static bool operator ==(Color a, Color b)
        {
            return a.value == b.value;
        }

        public static bool operator !=(Color a, Color b)
        {
            return !(a == b);
        }



        public static Color Black => new Color(0, 0, 0);
        public static Color White => new Color(255, 255, 255);

        public static Color Red => new Color(255, 0, 0);
        public static Color Green => new Color(0, 255, 0);
        public static Color Blue => new Color(0, 0, 255);

        public static Color Yellow => new Color(255, 255, 0);
        public static Color Cyan => new Color(0, 255, 255);
        public static Color Magenta => new Color(255, 0, 255);

        public static Color Gray => new Color(128, 128, 128);
        public static Color LightGray => new Color(192, 192, 192);
        public static Color DarkGray => new Color(64, 64, 64);

        public static Color Orange => new Color(255, 165, 0);
        public static Color Purple => new Color(128, 0, 128);
        public static Color Pink => new Color(255, 192, 203);

        public static Color Lime => new Color(50, 205, 50);
        public static Color Brown => new Color(139, 69, 19);
        public static Color Gold => new Color(255, 215, 0);
        public static Color Navy => new Color(0, 0, 128);
        public static Color Teal => new Color(0, 128, 128);

        public static Color Transparent => new Color(0, 0, 0, 0);

    }

}