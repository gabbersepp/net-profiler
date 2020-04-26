using System;

namespace NetProfiler.Logic
{
    public class SearchLogic
    {
        private int lastIndex = 0;
        private string lastSearch = string.Empty;
        private Type lastTarget;

        public int FindNextPosition(string search, string content, Type target)
        {
            int startIndex = lastSearch == search && lastTarget == target ? lastIndex : 0;
            lastTarget = target;
            lastSearch = search;

            var result = content.IndexOf(search, startIndex);
            lastIndex = result + 1;
            return result;
        }

        public int GetLineOf(string content, int index)
        {
            int line = 0;
            for (int i = 0; i < content.Length; i++)
            {
                if (i >= index)
                {
                    break;
                }

                if (content[i] == '\r')
                {
                    line++;
                }
            }

            return line;
        }
    }
}