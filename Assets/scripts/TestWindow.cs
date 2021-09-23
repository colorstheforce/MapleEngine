
using Editor;

public class TestWindow : EditorWindow
{
    [MenuItem("Custom/TestWindow")]
    public static void Init()
    {
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