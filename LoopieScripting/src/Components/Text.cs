namespace Loopie
{
    public class Text : Component
    {
        public string Value
        {
            get => GetText();
            set => SetText(value);
        }

        private string GetText()
        {
            
            return InternalCalls.Text_GetText(entity.ID, ID); ;
        }

        public void SetText(string text)
        {
            InternalCalls.Text_SetText(entity.ID, ID, text);
        }
    }
}