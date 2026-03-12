using System;
using System.Collections;
using System.Collections.Generic;

namespace Loopie
{
    public class YieldInstruction { }

    public class WaitForSeconds : YieldInstruction
    {
        public float TimeLeft;
        public WaitForSeconds(float seconds)
        {
            TimeLeft = seconds;
        }
    }

    class Coroutine
    {
        public IEnumerator Routine;
        public YieldInstruction Yielded;
    }

    public static class CoroutineSystem
    {
        static List<Coroutine> coroutines = new List<Coroutine>();

        public static void StartCoroutine(IEnumerator routine)
        {
            coroutines.Add(new Coroutine { Routine = routine, Yielded = null });
        }

        public static void UpdateCoroutines()
        {
            for (int i = coroutines.Count - 1; i >= 0; i--)
            {
                var c = coroutines[i];

                if (c.Yielded != null)
                {
                    if (c.Yielded is WaitForSeconds wait)
                    {
                        wait.TimeLeft -= Time.deltaTime;
                        if (wait.TimeLeft > 0)
                            continue;
                    }

                    c.Yielded = null;
                }

                if (!c.Routine.MoveNext())
                {
                    coroutines.RemoveAt(i);
                    continue;
                }

                var yielded = c.Routine.Current;

                if (yielded == null)
                {
                    c.Yielded = new YieldInstruction();
                }
                else if (yielded is YieldInstruction yi)
                {
                    c.Yielded = yi;
                }
            }
        }
    }
}