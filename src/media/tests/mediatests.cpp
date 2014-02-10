#include "scy/application.h"
#include "scy/logger.h"
#include "scy/signal.h"
#include "scy/queue.h"
#include "scy/packetqueue.h"
//#include "scy/syncdelegate.h"
#include "scy/media/flvmetadatainjector.h"
#include "scy/media/formatregistry.h"
#include "scy/media/mediafactory.h"
#include "scy/media/audiocapture.h"
#include "scy/media/avpacketencoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/fifo.h>
#include <libswscale/swscale.h>
}
/*
#include "scy/timer.h"
#include "scy/net/udpsocket.h"
#include "scy/net/tcpsocket.h" //Client


#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include <assert.h>
#include <stdio.h>


// Detect Memory Leaks
#ifdef _DEBUG
#include "MemLeakDetect/MemLeakDetect.h"
#include "MemLeakDetect/MemLeakDetect.cpp"
CMemLeakDetect memLeakDetect;
#endif
*/


using namespace std;
using namespace scy;
using namespace scy::av;
using namespace cv;


namespace scy {
namespace av {


class Tests
{		
public:
	Application& app;

	Tests(Application& app) : app(app)
	{   
		DebugL << "Running tests..." << endl;	

		try {		
			//runVideoCaptureThreadTest();

#if 0			
			//testVideoCapture();
			testVideoCaptureStream();

			//MediaFactory::instance().devices().print(cout);
			//audioCapture = new AudioCapture(device.id, 2, 44100);
			
			//videoCapture = new VideoCapture(0, true);
			//audioCapture = new AudioCapture(0, 1, 16000);	
			//audioCapture = new AudioCapture(0, 1, 11025);	
			//for (int i = 0; i < 100; i++) { //0000000000
			//	runAudioCaptureTest();
			//}
			
			//testDeviceManager();
			//runAudioCaptureThreadTest();
			//runAudioCaptureTest();
			//runOpenCVMJPEGTest();
			//runVideoRecorderTest();
			//runAudioRecorderTest();
			//runCaptureEncoderTest();
			//runAVEncoderTest();
			//runStreamEncoderTest();
			//runMediaSocketTest();
			//runFormatFactoryTest();
			//runMotionDetectorTest();
			//runBuildXMLString();
		
			//runStreamEncoderTest();
			//runOpenCVCaptureTest();
			//runDirectShowCaptureTest();
#endif
		}
		catch (std::exception& exc) {
			ErrorL << "Error: " << exc.what() << endl;
		}		
	};
	
		
	void testDeviceManager()
	{
		cout << "Starting" << endl;	

		Device device;
		if (MediaFactory::instance().devices().getDefaultVideoCaptureDevice(device)) {
			cout << "Default Video Device: " << device.id << ": " << device.name << endl;
		}
		if (MediaFactory::instance().devices().getDefaultAudioInputDevice(device)) {
			cout << "Default Audio Device: " << device.id << ": " << device.name << endl;
		}

		std::vector<Device> devices;
		if (MediaFactory::instance().devices().getVideoCaptureDevices(devices)) {			
			for (std::vector<Device>::const_iterator it = devices.begin(); it != devices.end(); ++it) {
				cout << "Printing Video Device: " << (*it).id << ": " << (*it).name << endl;
			}
		}
		if (MediaFactory::instance().devices().getAudioInputDevices(devices)) {			
			for (std::vector<Device>::const_iterator it = devices.begin(); it != devices.end(); ++it) {
				cout << "Printing Audio Device: " << (*it).id << ": " << (*it).name << endl;
			}
		}
	}
		
			
	// ---------------------------------------------------------------------
	// Video Capture Test
	//				
	void onVideoCaptureFrame(void* sender, av::MatrixPacket& packet)
	{
		DebugL << "On packet: " << packet.size() << endl;
			
		cv::imshow("VideoCaptureTest", *packet.mat);
	}	

	void testVideoCapture()
	{
		DebugL << "Starting" << endl;	

		av::VideoCapture::Ptr capture = MediaFactory::instance().createVideoCapture(0);
		capture->emitter += packetDelegate(this, &Tests::onVideoCaptureFrame);
		
		std::puts("Press any key to continue...");
		std::getchar();

		capture->emitter -= packetDelegate(this, &Tests::onVideoCaptureFrame);

		DebugL << "Complete" << endl;	
	}
			
			
	// ---------------------------------------------------------------------
	// Video Capture Test
	//				
	void onVideoCaptureStreamFrame(void* sender, av::MatrixPacket& packet)
	{
		DebugL << "On stream packet: " << packet.size() << endl;
	}	

	void testVideoCaptureStream()
	{
		DebugL << "Starting" << endl;	
		
		av::VideoCapture::Ptr capture = MediaFactory::instance().createVideoCapture(0);
		{
			PacketStream stream;		
			stream.emitter += packetDelegate(this, &Tests::onVideoCaptureStreamFrame);	
			stream.attachSource<av::VideoCapture>(capture, true);
			stream.start();
		
			std::puts("Press any key to continue...");
			std::getchar();
		}

		assert(capture->emitter.ndelegates() == 0);

		DebugL << "Complete" << endl;	
	}

		
	
	
	// ---------------------------------------------------------------------
	// Video Capture Thread Test
	//	
	class VideoCaptureThread: public async::Runnable
	{
	public:
		VideoCaptureThread(int deviceID, const std::string& name = "Capture Thread") : 
			_deviceID(deviceID),
			_name(name),
			closed(false),
			frames(0)
		{	
			//_thread.setName(name);			
			_thread.start(*this);
		}		

		VideoCaptureThread(const std::string& filename, const std::string& name = "Capture Thread") : 
			_filename(filename),
			_deviceID(0),
			_name(name),
			closed(false),
			frames(0)
		{
			//_thread.setName(name);
			_thread.start(*this);
		}	
		
		~VideoCaptureThread()
		{
			closed = true;
			_thread.join();
		}

		void run()
		{
#if 0
			VideoCapture::Ptr capture = !_filename.empty() ?
				MediaFactory::instance().createFileCapture(_filename) : 
				MediaFactory::instance().createVideoCapture(_deviceID);
#endif

			// Initialize the VideoCapture inside the thread context.
			// This will fail in windows unless the VideoCapture was
			// previously initialized in the main thread.
			VideoCapture::Ptr capture(!_filename.empty() ?
				new VideoCapture(_filename) : 
				new VideoCapture(_deviceID));

			capture->emitter += packetDelegate(this, &VideoCaptureThread::onVideo);	
			capture->start();

			while (!closed) {
				cv::waitKey(5);
			}		

			capture->emitter -= packetDelegate(this, &VideoCaptureThread::onVideo);	
			capture->stop();		
			
			DebugL << " Ended.................." << endl;
		}

		void onVideo(void* sender, MatrixPacket& packet)
		{
			DebugL << "On thread frame: " << packet.size() << endl;
			cv::imshow(_name, *packet.mat);
			frames++;
		}
		
		Thread _thread;
		std::string _filename;
		std::string _name;
		int	_deviceID;
		int frames;
		bool closed;
	};


	void runVideoCaptureThreadTest()
	{
		DebugL << "Running video capture test..." << endl;	

		
		// start and destroy multiple receivers
		std::list<VideoCaptureThread*> threads;

		try {
			for (int i = 0; i < 3; i++) {
				threads.push_back(new VideoCaptureThread(0));
			}		

			DebugL << " Ending.................." << endl;
		}
		catch (std::exception& exc) {
			ErrorL << "[VideoCaptureThread] Error: " << exc.what() << endl;
		}
		
		std::puts("Press any key to continue...");
		std::getchar();
		//util::pause();

		util::clearList(threads);
	}

#if 0
	// ---------------------------------------------------------------------
	// Packet Stream Encoder Test
	//
	static bool stopStreamEncoders;

	class StreamEncoderTest
	{
	public:
		StreamEncoderTest(const av::EncoderOptions& opts) : 
			closed(false), options(opts), frames(0), videoCapture(nullptr), audioCapture(nullptr)
		{
			//ofile.open("enctest1.mp4", ios::out | ios::binary);
			//assert(ofile.is_open());

			// Synchronize events and packet output with the default loop
			//stream.synchronizeOutput(uv::defaultLoop());
			//stream.setAsyncContext(std::make_shared<Idler>(uv::defaultLoop()));

			// Init captures
			if (options.oformat.video.enabled) {
				DebugL << "Video device: " << 0 << endl;
				videoCapture = MediaFactory::instance().createVideoCapture(0); //0
				//videoCapture = MediaFactory::instance().createFileCapture("D:/dev/lib/ffmpeg/bin/channel1.avi"); //0
				//videoCapture->emitter += packetDelegate(this, &StreamEncoderTest::onVideoCapture);
				videoCapture->getEncoderFormat(options.iformat);		
				stream.attachSource(videoCapture, true, true); //
				
				//options.iformat.video.pixelFmt = "yuv420p";
			}
			if (options.oformat.audio.enabled) {
				Device device;
				if (MediaFactory::instance().devices().getDefaultAudioInputDevice(device)) {
					DebugL << "Audio device: " << device.id << endl;
					audioCapture = MediaFactory::instance().createAudioCapture(device.id,
						options.oformat.audio.channels, 
						options.oformat.audio.sampleRate);
					stream.attachSource(audioCapture, true, true);
				}
				else assert(0);
			}			
								
			// Init as async queue for testing
			//stream.attach(new FPSLimiter(5), 4, true);
			//stream.attach(new AsyncPacketQueue, 2, true);
			//stream.attach(new AsyncPacketQueue, 3, true);
			//stream.attach(new AsyncPacketQueue, 4, true);

			// Init encoder				
			encoder = new AVPacketEncoder(options, 
				options.oformat.video.enabled && 
				options.oformat.audio.enabled);
			stream.attach(encoder, 5, true);

			// Start the stream
			stream.emitter += packetDelegate(this, &StreamEncoderTest::onVideoEncoded);	
			stream.StateChange += sdelegate(this, &StreamEncoderTest::onStreamStateChange);
			stream.start();
		}
		
		virtual ~StreamEncoderTest()
		{		
			DebugL << "Destroying" << endl;
			close();
			DebugL << "Destroying: OK" << endl;
		}

		void close()
		{
			DebugL << "########### Closing: " << frames << endl;
			closed = true;
			
			// Close the stream
			// This will flush any queued items
			//stream.stop();
			//stream.waitForSync();
			stream.close();
			
			// Make sure everything shutdown properly
			//assert(stream.queue().empty());
			//assert(encoder->isStopped());
			
			// Close the output file
			//ofile.close();
		}
			
		void onStreamStateChange(void* sender, PacketStreamState& state, const PacketStreamState& oldState)
		{
			DebugL << "########### On stream state change: " << oldState << " => " << state << endl;
		}

		void onVideoEncoded(void* sender, RawPacket& packet)
		{
			DebugL << "########### On packet: " << closed << ":" << packet.size() << endl;
			frames++;
			//assert(!closed);
			assert(packet.data());
			assert(packet.size());
			
			// Do not call stream::close from inside callback
			//ofile.write(packet.data(), packet.size());
			//assert(frames <= 3);
			//if (frames == 20)
			//	close();
		}
			
		void onVideoCapture(void* sender, av::MatrixPacket& packet)
		{
			DebugL << "On packet: " << packet.size() << endl;
			
			//cv::imshow("StreamEncoderTest", *packet.mat);
		}
		
		int frames;
		bool closed;
		PacketStream stream;
		VideoCapture::Ptr videoCapture;
		AudioCapture::Ptr audioCapture;
		//AsyncPacketQueue* queue;
		AVPacketEncoder* encoder;
		av::EncoderOptions options;
		//std::ofstream ofile;
	};	
		
	static void onShutdownSignal(void* opaque)
	{
		auto& tests = *reinterpret_cast<std::vector<StreamEncoderTest*>*>(opaque);

		stopStreamEncoders = true;

		for (unsigned i = 0; i < tests.size(); i++) {
			// Delete the pointer directly to 
			// ensure synchronization is golden.
			delete tests[i];
		}
	}

	void runStreamEncoderTest()
	{
		DebugL << "Running" << endl;	
		try
		{
			// Setup encoding format
			Format mp4(Format("MP4", "mp4", 
				VideoCodec("MPEG4", "mpeg4", 640, 480, 60), 
				//VideoCodec("H264", "libx264", 640, 480, 20)//,
				//AudioCodec("AAC", "aac", 2, 44100)
				//AudioCodec("MP3", "libmp3lame", 1, 8000, 64000)
				//AudioCodec("MP3", "libmp3lame", 2, 44100, 64000)
				AudioCodec("AC3", "ac3_fixed", 2, 44100, 64000)
			));

			Format mp3("MP3", "mp3", 
				AudioCodec("MP3", "libmp3lame", 1, 8000, 64000)); 
		
			//stopStreamEncoders = false;

			av::EncoderOptions options;		
			//options.ofile = "enctest.mp3";
			options.ofile = "itsanewday.mp4";	
			//options.ofile = "enctest.mjpeg";	
			options.oformat = mp4;

			// Initialize test runners
			int numTests = 1;
			std::vector<StreamEncoderTest*> threads;
			for (unsigned i = 0; i < numTests; i++) {
				threads.push_back(new StreamEncoderTest(options));
			}

			// Run until Ctrl-C is pressed
			//Application app;
			app.waitForShutdown(); //&threadsnullptr, nullptr
			
			for (unsigned i = 0; i < threads.size(); i++) {
				// Delete the pointer directly to 
				// ensure synchronization is golden.
				delete threads[i];
			}

			// Shutdown the garbage collector so we can free memory.
			//GarbageCollector::instance().shutdown();
			//DebugL << "#################### Finalizing" << endl;
			//GarbageCollector::instance().shutdown();
			//DebugL << "#################### Exiting" << endl;

			//DebugL << "#################### Finalizing" << endl;
			//app.cleanup();
			//DebugL << "#################### Exiting" << endl;

			// Wait for enter keypress
			//scy::pause();
		
			// Finalize the application to free all memory
			// Note: 2 tiny mem leaks (964 bytes) are from OpenCV
			//app.finalize();
		}
		catch (std::exception& exc)
		{
			ErrorL << "Error: " << exc.what() << endl;
			assert(0);
		}

		DebugL << "Ended" << endl;
	}

			
	// ---------------------------------------------------------------------
	// Capture Encoder Test
	//		
	class CaptureEncoder {
	public:
		CaptureEncoder(ICapture* capture, const av::EncoderOptions& options) : 
			capture(capture), encoder(options), closed(false) {
			assert(capture);	
			encoder.initialize();
		};

		void start()
		{
			capture->emitter += packetDelegate(this, &CaptureEncoder::onFrame);	
			capture->start();
		}

		void stop()
		{
			capture->emitter -= packetDelegate(this, &CaptureEncoder::onFrame);	
			capture->stop();
			encoder.uninitialize();
			closed = true;
		}

		void onFrame(void* sender, RawPacket& packet)
		{
			DebugL << "On packet: " << packet.size() << endl;
			assert(!closed);
			try 
			{	
				assert(0);
				//encoder.process(packet);
			}
			catch (std::exception& exc) 
			{
				ErrorL << "#######################: " << exc.what() << endl;
				stop();
			}
		}

		bool closed;
		ICapture* capture;
		AVEncoder encoder;
	};


	void runCaptureEncoderTest()
	{
		DebugL << "Starting" << endl;	
		
		/*
		av::VideoCapture capture(0);

		// Setup encoding format
		Format mp4(Format("MP4", "mp4", 
			VideoCodec("MPEG4", "mpeg4", 640, 480, 10)//, 
			//VideoCodec("H264", "libx264", 320, 240, 25),
			//AudioCodec("AAC", "aac", 2, 44100)
			//AudioCodec("MP3", "libmp3lame", 2, 44100, 64000)
			//AudioCodec("AC3", "ac3_fixed", 2, 44100, 64000)
		));
		
		av::EncoderOptions options;			
		options.ofile = "enctest.mp4"; // enctest.mjpeg
		options.oformat = mp4;
		setVideoCaptureInputFormat(&capture, options.iformat);	
		
		CaptureEncoder encoder(&capture, options);
		encoder.start();

		std::puts("Press any key to continue...");
		std::getchar();

		encoder.stop();
		*/

		DebugL << "Complete" << endl;	
	}


	// ---------------------------------------------------------------------
	// OpenCV Capture Test
	//	
	void runOpenCVCaptureTest()
	{
		DebugL << "Starting" << endl;	

		cv::VideoCapture cap(0);
		if (!cap.isOpened())
			assert(false);

		cv::Mat edges;
		cv::namedWindow("edges",1);
		for(;;) {
			cv::Mat frame;
			cap >> frame; // get a new frame from camera
			cv::cvtColor(frame, edges, CV_BGR2GRAY);
			cv::GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
			cv::Canny(edges, edges, 0, 30, 3);
			cv::imshow("edges", edges);
			if (cv::waitKey(30) >= 0) break;
		}

		DebugL << "Complete" << endl;	
	}
	
	
	// ---------------------------------------------------------------------
	// Audio Capture Thread Test
	//
	class AudioCaptureThread: public basic::Runnable
	{
	public:
		AudioCaptureThread(const std::string& name = "Capture Thread")
		{	
			_thread.setName(name);
			_thread.start(*this);
		}		
		
		~AudioCaptureThread()
		{
			_wakeUp.set();
			_thread.join();
		}

		void run()
		{
			try
			{
				//capture = new AudioCapture(0, 2, 44100);
				//capture = new AudioCapture(0, 1, 16000);	
				//capture = new AudioCapture(0, 1, 11025);	
				AudioCapture* capture = new AudioCapture(0, 1, 16000);
				capture->attach(audioDelegate(this, &AudioCaptureThread::onAudio));	
				
				_wakeUp.wait();
				
				capture->detach(audioDelegate(this, &AudioCaptureThread::onAudio));	
				delete capture;
				
				DebugL << "[AudioCaptureThread] Ending.................." << endl;
			}
			catch (std::exception& exc)
			{
				ErrorL << "[AudioCaptureThread] Error: " << exc.what() << endl;
			}
			
			DebugL << "[AudioCaptureThread] Ended.................." << endl;
			//delete this;
		}

		void onAudio(void* sender, AudioPacket& packet)
		{
			DebugL << "[AudioCaptureThread] On Packet: " << packet.size() << endl;
			//cv::imshow(_thread.name(), *packet.mat);
		}
		
		Thread	_thread;
		Poco::Event		_wakeUp;
		int				frames;
	};


	void runAudioCaptureThreadTest()
	{
		DebugL << "Running Audio Capture Thread test..." << endl;	
				
		// start and destroy multiple receivers
		list<AudioCaptureThread*> threads;
		for (int i = 0; i < 10; i++) { //0000000000
			threads.push_back(new AudioCaptureThread()); //Poco::format("Audio Capture Thread %d", i))
		}

		//util::pause();

		util::clearList(threads);
	}

	
	// ---------------------------------------------------------------------
	// Audio Capture Test
	//
	void onCaptureTestAudioCapture(void*, AudioPacket& packet)
	{
		DebugL << "onCaptureTestAudioCapture: " << packet.size() << endl;
		//cv::imshow("Target", *packet.mat);
	}	

	void runAudioCaptureTest()
	{
		DebugL << "Running Audio Capture test..." << endl;	
		
		AudioCapture* capture = new AudioCapture(0, 1, 16000);
		capture->attach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		//Util::pause();
		capture->detach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		delete capture;

		AudioCapture* capture = new AudioCapture(0, 1, 16000);
		capture->attach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));	
		//audioCapture->start();
		
		//Util::pause();

		capture->detach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		delete capture;
		//audioCapture->stop();
	}
	
	
	// ---------------------------------------------------------------------
	//
	// Video Media Socket Test
	//
	// ---------------------------------------------------------------------	

	class MediaConnection: public TCPServerConnection
	{
	public:
		MediaConnection(const StreamSocket& s) : 
		  TCPServerConnection(s), stop(false)//, lastTimestamp(0), timestampGap(0), waitForKeyFrame(true)
		{		
		}
		
		void run()
		{
			try
			{
				av::EncoderOptions options;
				//options.ofile = "enctest.mp4";
				//options.stopAt = time(0) + 3;
				av::setVideoCaptureInputForma(videoCapture, options.iformat);	
				//options.oformat = Format("MJPEG", "mjpeg", VideoCodec(Codec::MJPEG, "MJPEG", 1024, 768, 25));
				//options.oformat = Format("FLV", "flv", VideoCodec(Codec::H264, "H264", 400, 300, 25));	
				options.oformat = Format("FLV", "flv", VideoCodec(Codec::FLV, "FLV", 640, 480, 100));	
				//options.oformat = Format("FLV", "flv", VideoCodec(Codec::FLV, "FLV", 320, 240, 15));	
				//options.oformat = Format("FLV", "flv", VideoCodec(Codec::H264, "H264", 400, 300, 25));		
					

				//options.iformat.video.pixfmt = (scy::av::PixelFormat::ID)PIX_FMT_GRAY8; //PIX_FMT_BGR8; //PIX_FMT_BGR32 // PIX_FMT_BGR32
				//MotionDetector* detector = new MotionDetector();
				//detector->setVideoCapture(videoCapture);
				//stream.attach(detector, true);		
				//stream.attach(new SurveillanceMJPEGPacketizer(*detector), 20, true);	

				stream.attach(videoCapture, false);

				stream.attach(packetDelegate(this, &MediaConnection::onVideoEncoded));
		
				// init encoder				
				AVEncoder* encoder = new AVEncoder();
				encoder->setParams(options);
				encoder->initialize();
				stream.attach(encoder, 5, true);				
				
				//HTTPMultipartAdapter* packetizer = new HTTPMultipartAdapter("image/jpeg");
				//stream.attach(packetizer);

				//FLVMetadataInjector* injector = new FLVMetadataInjector(options.oformat);
				//stream.attach(injector);

				// start the stream
				stream.start();

				while (!stop)
				{
					Thread::sleep(50);
				}
				
				//stream.detach(packetDelegate(this, &MediaConnection::onVideoEncoded));
				//stream.stop();

				//outputFile.close();
				cerr << "MediaConnection: ENDING!!!!!!!!!!!!!" << endl;
			}
			catch (std::exception& exc)
			{
				cerr << "MediaConnection: " << exc.what() << endl;
			}
		}

		void onVideoEncoded(void* sender, RawPacket& packet)
		{
			StreamSocket& ss = socket();

			fpsCounter.tick();
			DebugL << "On Video Packet Encoded: " << fpsCounter.fps << endl;

			//if (fpsCounter.frames < 10)
			//	return;
			//if (fpsCounter.frames == 10) {
			//	stream.reset();
			//	return;
			//}

			try
			{		
				ss.sendBytes(packet.data, packet.size);
			}
			catch (std::exception& exc)
			{
				cerr << "MediaConnection: " << exc.what() << endl;
				stop = true;
			}
		}
		
		bool stop;
		PacketStream stream;
		FPSCounter fpsCounter;
	};

	void runMediaSocketTest()
	{		
		DebugL << "Running Media Socket Test" << endl;
		
		ServerSocket svs(666);
		TCPServer srv(new TCPServerConnectionFactoryImpl<MediaConnection>(), svs);
		srv.start();
		//util::pause();
	}

	
	// ---------------------------------------------------------------------
	// Video CaptureEncoder Test
	//
	//UDPSocket outputSocket;

	void runCaptureEncoderTest()
	{		
		DebugL << "Running Capture Encoder Test" << endl;	

		av::EncoderOptions options;
		options.ofile = "enctest.mp4";
		//options.stopAt = time(0) + 3;
		av::setVideoCaptureInputForma(videoCapture, options.iformat);	
		//options.oformat = Format("MJPEG", "mjpeg", VideoCodec(Codec::MJPEG, "MJPEG", 400, 300));
		options.oformat = Format("FLV", "flv", VideoCodec(Codec::H264, "H264", 400, 300, 25));	
		//options.oformat = Format("FLV", "flv", VideoCodec(Codec::FLV, "FLV", 320, 240, 15));	
				
		//CaptureEncoder<VideoEncoder> enc(videoCapture, options);	
		//encoder = new AVEncoder(stream.options());
		CaptureRecorder enc(options, videoCapture, audioCapture);		
		//enc.initialize();	
		
		enc.attach(packetDelegate(this, &Tests::onCaptureEncoderTestVideoEncoded));
		enc.start();
		//util::pause();
		enc.stop();

		DebugL << "Running Capture Encoder Test: END" << endl;	
	}
	
	FPSCounter fpsCounter;
	void onCaptureEncoderTestVideoEncoded(void* sender, MediaPacket& packet)
	{
		fpsCounter.tick();
		DebugL << "On Video Packet Encoded: " << fpsCounter.fps << endl;
	}

	// ---------------------------------------------------------------------
	// Video CaptureRecorder Test
	//
	void runVideoRecorderTest()
	{
		av::EncoderOptions options;
		options.ofile = "av_capture_test.flv";
		
		options.oformat = Format("FLV", "flv", 
			VideoCodec(Codec::FLV, "FLV", 320, 240, 15),
			//AudioCodec(Codec::NellyMoser, "NellyMoser", 1, 11025),
			AudioCodec(Codec::Speex, "Speex", 1, 16000)//,
			//AudioCodec(Codec::Speex, "Speex", 2, 44100)
			);	
		//options.oformat = Format("MP4", Format::MP4,
		options.oformat = Format("FLV", "flv",
			//VideoCodec(Codec::MPEG4, "MPEG4", 640, 480, 25), 
			//VideoCodec(Codec::H264, "H264", 640, 480, 25), 
			VideoCodec(Codec::FLV, "FLV", 640, 480, 25), 
			//AudioCodec(Codec::NellyMoser, "NellyMoser", 1, 11025)
			//AudioCodec(Codec::Speex, "Speex", 2, 44100)
			//AudioCodec(Codec::MP3, "MP3", 2, 44100)
			//AudioCodec(Codec::AAC, "AAC", 2, 44100)
			AudioCodec(Codec::AAC, "AAC", 1, 11025)
		);

		options.oformat = Format("M4A", Format::M4A,
			//AudioCodec(Codec::NellyMoser, "NellyMoser", 1, 44100)
			AudioCodec(Codec::AAC, "AAC", 2, 44100)
			//AudioCodec(Codec::AC3, "AC3", 2, 44100)
		);



		//options.stopAt = time(0) + 5; // Max 24 hours		
		av::setVideoCaptureInputForma(videoCapture, options.iformat);	

		CaptureRecorder enc(options, nullptr, audioCapture); //videoCapture
			
		audioCapture->start();	
		enc.start();
		//util::pause();
		enc.stop();
	}

	// ---------------------------------------------------------------------
	// Audio CaptureRecorder Test
	//
	void runAudioRecorderTest()
	{
		av::EncoderOptions options;
		options.ofile = "audio_test.mp3";
		options.stopAt = time(0) + 5;
		options.oformat = Format("MP3", "mp3",
			AudioCodec(Codec::MP3, "MP3", 2, 44100)
		);

		CaptureRecorder enc(options, nullptr, audioCapture);
		
		audioCapture->start();	
		enc.start();
		//util::pause();
		enc.stop();
	}
#endif
};


} // namespace av
} // namespace scy


#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <dbt.h>

/*
*/
//int main(int argc, char** argv)

int __stdcall _tWinMain(
                      HINSTANCE hInstanceExe, 
                      HINSTANCE, // should not reference this parameter
                      PTSTR lpstrCmdLine, 
                      int nCmdShow
                      )
{
	Logger::instance().add(new ConsoleChannel("debug", LTrace));
	//Logger::instance().setWriter(new AsyncLogWriter);		
	{
#ifdef _MSC_VER
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

		// Create the test application
		Application app;
	
		// Initialize the GarbageCollector in the main thread
		GarbageCollector::instance();

		// Preload our video captures in the main thread
		MediaFactory::instance().loadVideoCaptures();	
		{
			Tests run(app);
		}	

		// Shutdown the media factory and release devices
		MediaFactory::instance().unloadVideoCaptures();		
		MediaFactory::shutdown();	
	
		// Wait for user intervention before finalizing
		scy::pause();
			
		// Finalize the application to free all memory
		app.finalize();
	}
	
	// Cleanup singleton instances
	GarbageCollector::shutdown();
	Logger::shutdown();
	return 0;
}










/*

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <dbt.h>

#include <ks.h>
#include <ksmedia.h> // KSCATEGORY_*

// This GUID is for all USB serial host PnP drivers, but you can replace it 
// with any valid device class guid.
GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72, 
                      0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };

// For informational messages and window titles
PWSTR g_pszAppName;

// Forward declarations
void OutputMessage(HWND hOutWnd, WPARAM wParam, LPARAM lParam);
void ErrorHandler(LPTSTR lpszFunction);

//
// DoRegisterDeviceInterfaceToHwnd
//
BOOL DoRegisterDeviceInterfaceToHwnd( 
    IN GUID InterfaceClassGuid, 
    IN HWND hWnd,
    OUT HDEVNOTIFY *hDeviceNotify 
)
// Routine Description:
//     Registers an HWND for notification of changes in the device interfaces
//     for the specified interface class GUID. 

// Parameters:
//     InterfaceClassGuid - The interface class GUID for the device 
//         interfaces. 

//     hWnd - Window handle to receive notifications.

//     hDeviceNotify - Receives the device notification handle. On failure, 
//         this value is NULL.

// Return Value:
//     If the function succeeds, the return value is TRUE.
//     If the function fails, the return value is FALSE.

// Note:
//     RegisterDeviceNotification also allows a service handle be used,
//     so a similar wrapper function to this one supporting that scenario
//     could be made from this template.
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    *hDeviceNotify = RegisterDeviceNotification( 
        hWnd,                       // events recipient
        &NotificationFilter,        // type of device
        DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle //DEVICE_NOTIFY_ALL_INTERFACE_CLASSES //
        );

    if ( NULL == *hDeviceNotify ) 
    {
        ErrorHandler(TEXT("RegisterDeviceNotification"));
        return FALSE;
    }

    return TRUE;
}

//
// MessagePump
//
void MessagePump(
    HWND hWnd
)
// Routine Description:
//     Simple main thread message pump.
//

// Parameters:
//     hWnd - handle to the window whose messages are being dispatched

// Return Value:
//     None.
{
    MSG msg; 
    int retVal;

    // Get all messages for any window that belongs to this thread,
    // without any filtering. Potential optimization could be
    // obtained via use of filter values if desired.

    while( (retVal = GetMessage(&msg, NULL, 0, 0)) != 0 ) 
    { 
        if ( retVal == -1 )
        {
            ErrorHandler(TEXT("GetMessage"));
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } 
}

//
// WinProcCallback
//
INT_PTR WINAPI WinProcCallback(
                              HWND hWnd,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam
                              )
// Routine Description:
//     Simple Windows callback for handling messages.
//     This is where all the work is done because the example
//     is using a window to process messages. This logic would be handled 
//     differently if registering a service instead of a window.

// Parameters:
//     hWnd - the window handle being registered for events.

//     message - the message being interpreted.

//     wParam and lParam - extended information provided to this
//          callback by the message sender.

//     For more information regarding these parameters and return value,
//     see the documentation for WNDCLASSEX and CreateWindowEx.
{
    LRESULT lRet = 1;
    static HDEVNOTIFY hDeviceNotify;
    static HWND hEditWnd;
    static ULONGLONG msgCount = 0;

    switch (message)
    {
    case WM_CREATE:
        //
        // This is the actual registration., In this example, registration 
        // should happen only once, at application startup when the window
        // is created.
        //
        // If you were using a service, you would put this in your main code 
        // path as part of your service initialization.
        //
        if ( ! DoRegisterDeviceInterfaceToHwnd(
                        KSCATEGORY_VIDEO, //WceusbshGUID, 
                        hWnd,
                        &hDeviceNotify) )
        {
            // Terminate on failure.
            ErrorHandler(TEXT("DoRegisterDeviceInterfaceToHwnd"));
            ExitProcess(1);
        }


        //
        // Make the child window for output.
        //
        hEditWnd = CreateWindow(TEXT("EDIT"),// predefined class 
                                NULL,        // no window title 
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
                                ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 
                                0, 0, 0, 0,  // set size in WM_SIZE message 
                                hWnd,        // parent window 
                                (HMENU)1,    // edit control ID 
                                (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE), 
                                NULL);       // pointer not needed 

        if ( hEditWnd == NULL )
        {
            // Terminate on failure.
            ErrorHandler(TEXT("CreateWindow: Edit Control"));
            ExitProcess(1);
        }
        // Add text to the window. 
        SendMessage(hEditWnd, WM_SETTEXT, 0, 
            (LPARAM)TEXT("Registered for USB device notification...\n")); 

        break;

    case WM_SETFOCUS: 
        SetFocus(hEditWnd); 

        break;

    case WM_SIZE: 
        // Make the edit control the size of the window's client area. 
        MoveWindow(hEditWnd, 
                   0, 0,                  // starting x- and y-coordinates 
                   LOWORD(lParam),        // width of client area 
                   HIWORD(lParam),        // height of client area 
                   TRUE);                 // repaint window 

        break;

    case WM_DEVICECHANGE:
    {
        //
        // This is the actual message from the interface via Windows messaging.
        // This code includes some additional decoding for this particular device type
        // and some common validation checks.
        //
        // Note that not all devices utilize these optional parameters in the same
        // way. Refer to the extended information for your particular device type 
        // specified by your GUID.
        //
        PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;
        TCHAR strBuff[256];

        // Output some messages to the window.
        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: DBT_DEVICEARRIVAL\n"), msgCount);
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: DBT_DEVICEREMOVECOMPLETE\n"), msgCount);
            break;
        case DBT_DEVNODES_CHANGED:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: DBT_DEVNODES_CHANGED\n"), msgCount);
            break;
        default:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: WM_DEVICECHANGE message received, value %d unhandled.\n"), 
                msgCount, wParam);
            break;
        }
        OutputMessage(hEditWnd, wParam, (LPARAM)strBuff);
    }
            break;
    case WM_CLOSE:
        if ( ! UnregisterDeviceNotification(hDeviceNotify) )
        {
           ErrorHandler(TEXT("UnregisterDeviceNotification")); 
        }
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        // Send all other messages on to the default windows handler.
        lRet = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return lRet;
}

#define WND_CLASS_NAME TEXT("SampleAppWindowClass")

//
// InitWindowClass
//
BOOL InitWindowClass()
// Routine Description:
//      Simple wrapper to initialize and register a window class.

// Parameters:
//     None

// Return Value:
//     TRUE on success, FALSE on failure.

// Note: 
//     wndClass.lpfnWndProc and wndClass.lpszClassName are the
//     important unique values used with CreateWindowEx and the
//     Windows message pump.
{
    WNDCLASSEX wndClass;

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
    wndClass.lpfnWndProc = reinterpret_cast<WNDPROC>(WinProcCallback);
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hIcon = LoadIcon(0,IDI_APPLICATION);
    wndClass.hbrBackground = CreateSolidBrush(RGB(192,192,192));
    wndClass.hCursor = LoadCursor(0, IDC_ARROW);
    wndClass.lpszClassName = WND_CLASS_NAME;
    wndClass.lpszMenuName = NULL;
    wndClass.hIconSm = wndClass.hIcon;


    if ( ! RegisterClassEx(&wndClass) )
    {
        ErrorHandler(TEXT("RegisterClassEx"));
        return FALSE;
    }
    return TRUE;
}

//
// main
//

int __stdcall _tWinMain(
                      HINSTANCE hInstanceExe, 
                      HINSTANCE, // should not reference this parameter
                      PTSTR lpstrCmdLine, 
                      int nCmdShow
                      )
{
//
// To enable a console project to compile this code, set
// Project->Properties->Linker->System->Subsystem: Windows.
//

    int nArgC = 0;
    PWSTR* ppArgV = CommandLineToArgvW(lpstrCmdLine, &nArgC);
    g_pszAppName = ppArgV[0];

    if ( ! InitWindowClass() )
    {
        // InitWindowClass displays any errors
        return -1;
    }

    // Main app window

    HWND hWnd = CreateWindowEx(
                    WS_EX_CLIENTEDGE | WS_EX_APPWINDOW,
                    WND_CLASS_NAME,
                    g_pszAppName,
                    WS_OVERLAPPEDWINDOW, // style
                    CW_USEDEFAULT, 0, 
                    640, 480,
                    NULL, NULL, 
                    hInstanceExe, 
                    NULL);
    
    if ( hWnd == NULL )
    {
        ErrorHandler(TEXT("CreateWindowEx: main appwindow hWnd"));
        return -1;
    }

    // Actually draw the window.

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    // The message pump loops until the window is destroyed.

    MessagePump(hWnd);

    return 1;
}

//
// OutputMessage
//
void OutputMessage(
    HWND hOutWnd, 
    WPARAM wParam, 
    LPARAM lParam
)
// Routine Description:
//     Support routine.
//     Send text to the output window, scrolling if necessary.

// Parameters:
//     hOutWnd - Handle to the output window.
//     wParam  - Standard windows message code, not used.
//     lParam  - String message to send to the window.

// Return Value:
//     None

// Note:
//     This routine assumes the output window is an edit control
//     with vertical scrolling enabled.

//     This routine has no error checking.
{
    LRESULT   lResult;
    LONG      bufferLen;
    LONG      numLines;
    LONG      firstVis;
  
    // Make writable and turn off redraw.
    lResult = SendMessage(hOutWnd, EM_SETREADONLY, FALSE, 0L);
    lResult = SendMessage(hOutWnd, WM_SETREDRAW, FALSE, 0L);

    // Obtain current text length in the window.
    bufferLen = SendMessage (hOutWnd, WM_GETTEXTLENGTH, 0, 0L);
    numLines = SendMessage (hOutWnd, EM_GETLINECOUNT, 0, 0L);
    firstVis = SendMessage (hOutWnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
    lResult = SendMessage (hOutWnd, EM_SETSEL, bufferLen, bufferLen);

    // Write the new text.
    lResult = SendMessage (hOutWnd, EM_REPLACESEL, 0, lParam);

    // See whether scrolling is necessary.
    if (numLines > (firstVis + 1))
    {
        int        lineLen = 0;
        int        lineCount = 0;
        int        charPos;

        // Find the last nonblank line.
        numLines--;
        while(!lineLen)
        {
            charPos = SendMessage(
                hOutWnd, EM_LINEINDEX, (WPARAM)numLines, 0L);
            lineLen = SendMessage(
                hOutWnd, EM_LINELENGTH, charPos, 0L);
            if(!lineLen)
                numLines--;
        }
        // Prevent negative value finding min.
        lineCount = numLines - firstVis;
        lineCount = (lineCount >= 0) ? lineCount : 0;
        
        // Scroll the window.
        lResult = SendMessage(
            hOutWnd, EM_LINESCROLL, 0, (LPARAM)lineCount);
    }

    // Done, make read-only and allow redraw.
    lResult = SendMessage(hOutWnd, WM_SETREDRAW, TRUE, 0L);
    lResult = SendMessage(hOutWnd, EM_SETREADONLY, TRUE, 0L);
}  

//
// ErrorHandler
//
void ErrorHandler(
   LPTSTR lpszFunction
) 
// Routine Description:
//     Support routine.
//     Retrieve the system error message for the last-error code
//     and pop a modal alert box with usable info.

// Parameters:
//     lpszFunction - String containing the function name where 
//     the error occurred plus any other relevant data you'd 
//     like to appear in the output. 

// Return Value:
//     None

// Note:
//     This routine is independent of the other windowing routines
//     in this application and can be used in a regular console
//     application without modification.
{ 

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process.

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)
                  + lstrlen((LPCTSTR)lpszFunction)+40)
                  * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, g_pszAppName, MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

*/