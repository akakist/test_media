#include "MFPipeImpl.h"
#include <iostream>
#include <unistd.h>

#define PACKETS_COUNT	(8)

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(arr)	(sizeof(arr)/sizeof((arr)[0]))
#endif // SIZEOF_ARRAY

std::string dump(const std::vector<uint8_t> &v)
{
    std::string out;
    for(auto z:v)
    {
        out+=std::to_string(z)+" ";
    }
    return out;
}


int TestMethod1()
{
//    std::vector<REF_getter<MF_BASE_TYPE> > arrBuffersIn;
//    for (int i = 0; i < PACKETS_COUNT; ++i)
    {
        //size_t cbSize = 128 * 1024 + rand() % (256 * 1024);
//        auto b=new
//        arrBuffersIn.push_back(new MF_BUFFER);

//		arrBuffersIn[i]->flags = eMFBF_Buffer;
        // Fill buffer
        // TODO: fill by test data here
    }

//	std::string pstrEvents[] = { "event1", "event2", "event3", "event4", "event5", "event6", "event7", "event8" };
//	std::string pstrMessages[] = { "message1", "message2", "message3", "message4", "message5", "message6", "message7", "message8" };


    //////////////////////////////////////////////////////////////////////////
    // Pipe creation

    // Write pipe
    MFPipeImpl MFPipe_Write;
    MFPipe_Write.PipeCreate("udp://127.0.0.1:12345", "");

    // Read pipe
    MFPipeImpl MFPipe_Read;
    MFPipe_Read.PipeOpen("udp://127.0.0.1:12345", 32, "");

    //////////////////////////////////////////////////////////////////////////
    // Test code (single-threaded)
    // TODO: multi-threaded

    // Note: channels ( "ch1", "ch2" is optional)

    for (int i = 0; i < 64; ++i)
    {
        {
            auto b=new MF_BUFFER;
            for(int j=0; j<10; j++)
            {
                b->data.push_back(j+i);
            }
            REF_getter<MF_BASE_TYPE> z=b;
            MFPipe_Write.PipePut("ch1", z, 0, "");
        }
    }
    for (int i = 0; i < 64; ++i)
    {
        {
            auto b=new MF_FRAME;
            for(int j=0; j<10; j++)
            {
                b->vec_audio_data.push_back(j+i);
            }
            REF_getter<MF_BASE_TYPE> z=b;
            MFPipe_Write.PipePut("ch1", z, 0, "");
        }
    }
    sleep(1);
    for (int i = 0; i < 128; ++i)
    {
        REF_getter<MF_BASE_TYPE> z(NULL);
        MFPipe_Read.PipeGet("ch1",z,100,"");
        {
            MF_FRAME *f=dynamic_cast<MF_FRAME*>(z.operator->());
            if(f)
            {

                printf("MF_FRAME::vec_audio_data %s\n",dump(f->vec_audio_data).c_str());
                continue;
            }
        }
        {
            MF_BUFFER *b=dynamic_cast<MF_BUFFER*>(z.operator->());
            if(b)
            {
                printf("MF_BUFFER::data %s\n",dump(b->data).c_str());
                continue;
            }

        }
        printf("MF_BASE_TYPE invalid\n");
    }


//		MFPipe_Write.PipePut("ch1", arrBuffersIn[i % PACKETS_COUNT], 0, "");
//		MFPipe_Write.PipePut("ch2", arrBuffersIn[(i + 1) % PACKETS_COUNT], 0, "");
//		MFPipe_Write.PipeMessagePut("ch1", pstrEvents[i % PACKETS_COUNT], pstrMessages[i % PACKETS_COUNT], 100);
//		MFPipe_Write.PipeMessagePut("ch2", pstrEvents[(i + 1) % PACKETS_COUNT], pstrMessages[(i + 1) % PACKETS_COUNT], 100);

//		MFPipe_Write.PipePut("ch1", arrBuffersIn[i % PACKETS_COUNT], 0, "");
//		MFPipe_Write.PipePut("ch2", arrBuffersIn[(i + 1) % PACKETS_COUNT], 0, "");

//		std::string strPipeName;
//		MFPipe_Write.PipeInfoGet(&strPipeName, "", NULL);

//		MFPipe::MF_PIPE_INFO mfInfo = {};
//		MFPipe_Write.PipeInfoGet(NULL, "ch2", &mfInfo);
//		MFPipe_Write.PipeInfoGet(NULL, "ch1", &mfInfo);

//		// Read from pipe
//		std::string arrStrings[4];

//		MFPipe_Write.PipeMessageGet("ch1", &arrStrings[0], &arrStrings[1], 100);
//		MFPipe_Write.PipeMessageGet("ch2", &arrStrings[2], &arrStrings[3], 100);
//		MFPipe_Write.PipeMessageGet("ch2", NULL, &arrStrings[2], 100);

//        REF_getter<MF_BASE_TYPE> arrBuffersOut[8];
//		MFPipe_Write.PipeGet("ch1", arrBuffersOut[0], 100, "");
//		MFPipe_Write.PipeGet("ch2", arrBuffersOut[1], 100, "");

//		MFPipe_Write.PipeGet("ch1", arrBuffersOut[4], 100, "");
//		MFPipe_Write.PipeGet("ch2", arrBuffersOut[5], 100, "");
//		MFPipe_Write.PipeGet("ch2", arrBuffersOut[6], 100, "");

    // TODO: Your test code here
    return 0;
}

//	return 0;
//}

int main(void)
{
    if(TestMethod1())
    {
        std::cerr << "TestMethod1: Failed" << std::endl;
        return 1;
    }

    return 0;
}
