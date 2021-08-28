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

    abstract class EditorWindow 
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
            if (!windowCache.ContainsKey(typeof(T).FullName)) {
                windowCache.Add(typeof(T).FullName, System.Activator.CreateInstance<T>());
            }
            return (T)windowCache[typeof(T).FullName];
        }
        private static Dictionary<string, EditorWindow> windowCache;
    };

  
    class TestWindow : EditorWindow
    {
        [MenuItem("Custom/TestWindow")]
        public static void Init() { 
        	GetWindow<TestWindow>();
        }

        public override void OnImGui()
        {
            ImGuiNET.ImGui.Begin("Test");

            ImGuiNET.ImGui.End();
        }

        public override void OnUpdate(float dt)
        {
        }


    }

}
