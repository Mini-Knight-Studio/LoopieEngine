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

    public class Coroutine
    {
        public string OwnerID;
        public IEnumerator Routine;
        public YieldInstruction Yielded;
    }

    internal static class CoroutineSystem
    {
        static List<Coroutine> coroutines = new List<Coroutine>();

        internal static Coroutine StartCoroutine(string ownerID, IEnumerator routine)
        {
            Coroutine coroutine = new Coroutine {OwnerID = ownerID, Routine = routine, Yielded = null };
            coroutines.Add(coroutine);
            return coroutine;
        }

        internal static void StopCoroutine(Coroutine coroutine)
        {
            coroutines.Remove(coroutine);
        }

        internal static void StopCoroutinesByOwner(string ownerID)
        {
            for (int i = coroutines.Count - 1; i >= 0; i--)
            {
                if (coroutines[i].OwnerID== ownerID)
                {
                    coroutines.RemoveAt(i);
                }
            }
        }

        internal static void StopAllCoroutines()
        {
            coroutines.Clear();
        }

        internal static void UpdateCoroutines()
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