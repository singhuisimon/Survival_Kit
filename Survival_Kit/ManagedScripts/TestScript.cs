using ScriptAPI;

public class TestScript : ScriptAPI.Script
{
    public override void Update()
    {
        TransformComponent tf = GetTransformComponent();
        tf.X += 0.01f;
        Console.WriteLine($"x: {tf.X}");
    }
}