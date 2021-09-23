using System.Collections.Generic;

namespace Editor
{
    [System.AttributeUsage(System.AttributeTargets.Method)]
    public class MenuItem : System.Attribute
    {
        private string path;

        public MenuItem(string path)
        {
            this.Path = path;
        }
        public string Path { get => path; set => path = value; }
    }

    public abstract class EditorWindow 
    {
        public abstract void OnUpdate(float dt);
        public abstract void OnImGui();

        //call from native       
        public static void Update(float dt) 
        {
            foreach (var window in windowCache) {
                window.Value.OnUpdate(dt);
            }
        }
        //call from native
        public static void ImGui() 
        {
            foreach (var window in windowCache)
            {
                window.Value.OnImGui();
            }
        }

        public static T GetWindow<T> () where T : EditorWindow
        {
            var name = typeof(T).FullName;
            Maple.Debug.LogV(name);
            if (!windowCache.ContainsKey(name)) {
                windowCache.Add(name, System.Activator.CreateInstance<T>());
            }
            return (T)windowCache[name];
        }
        private static Dictionary<string, EditorWindow> windowCache = new Dictionary<string, EditorWindow>();
    };

}
