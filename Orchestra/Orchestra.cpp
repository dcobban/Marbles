#include <marbles.h>
#include <platform/window.h>
#include <platform/device.h>
#include <iostream>

marbles::device* idealDevice(marbles::device::manager* manager)
{
    (void)manager;
    return nullptr;
}

int main(int argc, char* argv[])
{
    marbles::device::manager manager("Orchestra", 0, true);

    (void)argc; (void)argv;
    marbles::window root;
    marbles::window::builder builder;
    marbles::device* myDevice = idealDevice(&manager);
    builder.bind(myDevice);
    builder.create(&root);
    while (root.is_open())
    {
        root.poll();
    }

    return 0;
}