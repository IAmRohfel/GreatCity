#include "ApplicationCore/Application.h"

int main()
{
    GCApplication_Create();
    GCApplication_Run();
    GCApplication_Destroy();

    return 0;
}