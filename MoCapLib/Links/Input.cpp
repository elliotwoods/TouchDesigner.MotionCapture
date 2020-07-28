#include "pch_MoCapLib.h"
#include "Input.h"
#include "OutputsRegister.h"

namespace TD_MoCap {
	namespace Links {
		//----------
		Input::~Input()
		{
			this->unsubscribe();
		}

		//----------
		void
			Input::update(const OP_DATInput* dat)
		{
			Output::ID newID;

			if (!Output::getIDFromDAT(dat, newID)) {
				this->unsubscribe();
			}

			// if we stayed the same, ignore it
			if (this->connectedOutput) {
				if (connectedOutput->getID() == newID) {
					return;
				}
			}

			this->subscribe(newID);
		}

		//----------
		void
			Input::subscribe(Output::ID id)
		{
			this->unsubscribe();

			auto output = OutputsRegister::X().findByID(id);
			if (output) {
				this->connectedOutput = output;
				output->addSubscriber(this);
			}
		}

		//----------
		void
			Input::unsubscribe(bool informOutput)
		{
			if (!this->connectedOutput) {
				return;
			}
			else {
				if (informOutput) {
					this->connectedOutput->removeSubscriber(this);
				}
				this->channel.clear();
				this->connectedOutput = nullptr;
			}
		}

		//----------
		bool
			Input::isConnected() const
		{
			return (bool)this->connectedOutput;
		}

		//----------
		void
			Input::close()
		{
			this->channel.close();
		}

		//----------
		std::shared_ptr<Frames::BaseFrame>
			Input::receiveNextFrameWait(const std::chrono::system_clock::duration& timeout)
		{
			std::shared_ptr<Frames::BaseFrame> frame;

			this->channel.tryReceive(frame, timeout);

			if (frame) {
				this->lastFrame = frame;
			}

			return frame;
		}

		//----------
		std::shared_ptr<Frames::BaseFrame>
			Input::receiveNextFrameDontWait()
		{
			std::shared_ptr<Frames::BaseFrame> frame;

			this->channel.tryReceive(frame);

			if (frame) {
				this->lastFrame = frame;
			}

			return frame;
		}

		//----------
		std::shared_ptr<Frames::BaseFrame>
			Input::receiveLatestFrame(bool useCached)
		{
			std::shared_ptr<Frames::BaseFrame> frame;
			while (this->channel.tryReceive(frame)) {
				// flush until latest	
			}

			if (frame) {
				this->lastFrame = frame;
				return frame;
			}
			else if (useCached) {
				return lastFrame;
			}
			else {
				return nullptr;
			}
		}

		//----------
		std::shared_ptr<Frames::BaseFrame>
			Input::getLastFrame()
		{
			return this->lastFrame;
		}

		//----------
		Utils::ThreadChannel<std::shared_ptr<Frames::BaseFrame>>&
			Input::getChannel()
		{
			return this->channel;
		}

		//----------
		size_t
			Input::getDroppedFrameCount() const
		{
			return this->droppedFrameCount;
		}

		//----------
		void
			Input::send(std::shared_ptr<Frames::BaseFrame> frame)
		{
			if (this->channel.size() > this->maxSize) {
				this->droppedFrameCount++;
			}
			else {
				this->channel.send(frame);
			}
		}
	}
}