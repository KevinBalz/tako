# Input System (WIP)

You can define an `InputAction` and bind it to different controls, like keyboard or gamepad buttons.

```cpp
InputAction jumpAction = input->CreateAction("Jump");
input->Bind(jumpAction, Key::Space);
input->Bind(jumpAction, GamepadButton::A);
```
You can then poll the current state of the action

```cpp
if (input->GetActionDown(jumpAction))
{
    // Will get executed after the player
    // presses either `Space` on the keybord
    // or `A` on an Xbox Controller
    Jump();
}
```