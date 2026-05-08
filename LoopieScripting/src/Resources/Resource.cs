using System;

namespace Loopie
{
    public abstract class Resource
    {
        public string ID { get; internal set; }
        public int Index { get; internal set; }
    }
}