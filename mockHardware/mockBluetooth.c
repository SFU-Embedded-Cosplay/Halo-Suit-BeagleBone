/* **************************************
 * mockBluetooth.c
 * emulates communication that would normally happen over bluetooth
 */
#ifdef MOCK_BLUETOOTH

int beagleblue_glass_send(char *buffer)
{
   return 0;
}

int beagleblue_android_send(char *buffer)
{
    return 0;
}

void beagleblue_init(void (*on_receive)(char *buffer)) 
{
    return NULL;
}

void beagleblue_exit() 
{
}

void beagleblue_join()
{
}

#endif
