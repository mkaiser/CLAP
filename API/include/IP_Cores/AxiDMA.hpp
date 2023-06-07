/* 
 *  File: AxiDMA.hpp
 *  Copyright (c) 2023 Florian Porrmann
 *  
 *  MIT License
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 */

#pragma once

#include "../internal/RegisterControl.hpp"
#include "internal/WatchDog.hpp"

#include <cstdint>

namespace clap
{
template<typename T>
class AxiDMA : public internal::RegisterControlBase
{
	DISABLE_COPY_ASSIGN_MOVE(AxiDMA)
	enum REGISTER_MAP
	{
		MM2S_DMACR        = 0x00,
		MM2S_DMASR        = 0x04,
		MM2S_CURDESC      = 0x08,
		MM2S_CURDESC_MSB  = 0x0C,
		MM2S_TAILDESC     = 0x10,
		MM2S_TAILDESC_MSB = 0x14,
		MM2S_SA           = 0x18,
		MM2S_SA_MSB       = 0x1C,
		MM2S_LENGTH       = 0x28,
		SG_CTL            = 0x2C,
		S2MM_DMACR        = 0x30,
		S2MM_DMASR        = 0x34,
		S2MM_CURDESC      = 0x38,
		S2MM_CURDESC_MSB  = 0x3C,
		S2MM_TAILDESC     = 0x40,
		S2MM_TAILDESC_MSB = 0x44,
		S2MM_DA           = 0x48,
		S2MM_DA_MSB       = 0x4C,
		S2MM_LENGTH       = 0x58
	};

public:
	enum DMAInterrupts
	{
		INTR_ON_COMPLETE = 1 << 0,
		INTR_ON_DELAY    = 1 << 1,
		INTR_ON_ERROR    = 1 << 2,
		INTR_ALL         = (1 << 3) - 1 // All bits set
	};

public:
	AxiDMA(const CLAPPtr& pClap, const uint64_t& ctrlOffset) :
		RegisterControlBase(pClap, ctrlOffset),
		m_watchDogMM2S("AxiDMA_MM2S", pClap->MakeUserInterrupt()),
		m_watchDogS2MM("AxiDMA_S2MM", pClap->MakeUserInterrupt())
	{
		registerReg<uint32_t>(m_mm2sCtrlReg, MM2S_DMACR);
		registerReg<uint32_t>(m_mm2sStatReg, MM2S_DMASR);
		registerReg<uint32_t>(m_s2mmCtrlReg, S2MM_DMACR);
		registerReg<uint32_t>(m_s2mmStatReg, S2MM_DMASR);

		UpdateAllRegisters();

		m_watchDogMM2S.SetStatusRegister(&m_mm2sStatReg);
		m_watchDogS2MM.SetStatusRegister(&m_s2mmStatReg);
	}

	////////////////////////////////////////

	// Starts both channels
	void Start(const T& srcAddr, const uint32_t& srcLength, const T& dstAddr, const uint32_t& dstLength)
	{
		Start(DMAChannel::MM2S, srcAddr, srcLength);
		Start(DMAChannel::S2MM, dstAddr, dstLength);
	}

	void Start(const Memory& srcMem, const Memory& dstMem)
	{
		Start(static_cast<T>(srcMem.GetBaseAddr()), static_cast<uint32_t>(srcMem.GetSize()),
			  static_cast<T>(dstMem.GetBaseAddr()), static_cast<uint32_t>(dstMem.GetSize()));
	}

	void Start(const DMAChannel& channel, const Memory& mem)
	{
		Start(channel, static_cast<T>(mem.GetBaseAddr()), static_cast<uint32_t>(mem.GetSize()));
	}

	// Starts the specified channel
	void Start(const DMAChannel& channel, const T& addr, const uint32_t& length)
	{
		if (channel == DMAChannel::MM2S)
		{
			if (!m_watchDogMM2S.Start())
			{
				LOG_ERROR << CLASS_TAG("AxiDMA") << "Watchdog for MM2S already running!" << std::endl;
				return;
			}

			m_mm2sStatReg.Reset();

			// Set the RunStop bit
			m_mm2sCtrlReg.Start();
			// Set the source address
			setMM2SSrcAddr(addr);
			// Set the amount of bytes to transfer
			setMM2SByteLength(length);
		}

		if (channel == DMAChannel::S2MM)
		{
			if (!m_watchDogS2MM.Start())
			{
				LOG_ERROR << CLASS_TAG("AxiDMA") << "Watchdog for S2MM already running!" << std::endl;
				return;
			}

			m_s2mmStatReg.Reset();

			// Set the RunStop bit
			m_s2mmCtrlReg.Start();
			// Set the destination address
			setS2MMDestAddr(addr);
			// Set the amount of bytes to transfer
			setS2MMByteLength(length);
		}
	}

	// Stops both channel
	void Stop()
	{
		Stop(DMAChannel::MM2S);
		Stop(DMAChannel::S2MM);
	}

	// Stops the specified channel
	void Stop(const DMAChannel& channel)
	{
		// Unset the RunStop bit
		if (channel == DMAChannel::MM2S)
			m_mm2sCtrlReg.Stop();
		else
			m_s2mmCtrlReg.Stop();
	}

	bool WaitForFinish(const DMAChannel& channel, const int32_t& timeoutMS = WAIT_INFINITE)
	{
		if (channel == DMAChannel::MM2S)
			return m_watchDogMM2S.WaitForFinish(timeoutMS);
		else
			return m_watchDogS2MM.WaitForFinish(timeoutMS);
	}

	////////////////////////////////////////

	////////////////////////////////////////

	void Reset()
	{
		Reset(DMAChannel::MM2S);
		Reset(DMAChannel::S2MM);
	}

	void Reset(const DMAChannel& channel)
	{
		if (channel == DMAChannel::MM2S)
			m_mm2sCtrlReg.DoReset();
		else
			m_s2mmCtrlReg.DoReset();
	}

	////////////////////////////////////////

	////////////////////////////////////////

	void EnableInterrupts(const uint32_t& eventNoMM2S, const uint32_t& eventNoS2MM, const DMAInterrupts& intr = INTR_ALL)
	{
		EnableInterrupts(DMAChannel::MM2S, eventNoMM2S, intr);
		EnableInterrupts(DMAChannel::S2MM, eventNoS2MM, intr);
	}

	void EnableInterrupts(const DMAChannel& channel, const uint32_t& eventNo, const DMAInterrupts& intr = INTR_ALL)
	{
		if (channel == DMAChannel::MM2S)
		{
			m_mm2sCtrlReg.Update();
			m_watchDogMM2S.InitInterrupt(getDevNum(), eventNo, &m_mm2sStatReg);
			m_mm2sCtrlReg.EnableInterrupts(intr);
		}
		else
		{
			m_s2mmCtrlReg.Update();
			m_watchDogS2MM.InitInterrupt(getDevNum(), eventNo, &m_s2mmStatReg);
			m_s2mmCtrlReg.EnableInterrupts(intr);
		}
	}

	void DisableInterrupts(const DMAInterrupts& intr = INTR_ALL)
	{
		DisableInterrupts(DMAChannel::MM2S, intr);
		DisableInterrupts(DMAChannel::S2MM, intr);
	}

	void DisableInterrupts(const DMAChannel& channel, const DMAInterrupts& intr = INTR_ALL)
	{
		if (channel == DMAChannel::MM2S)
		{
			m_watchDogMM2S.UnsetInterrupt();
			m_mm2sCtrlReg.DisableInterrupts(intr);
		}
		else
		{
			m_watchDogS2MM.UnsetInterrupt();
			m_s2mmCtrlReg.DisableInterrupts(intr);
		}
	}

	////////////////////////////////////////

	////////////////////////////////////////

	T GetMM2SSrcAddr()
	{
		return readRegister<T>(MM2S_SA);
	}

	T GetS2MMDestAddr()
	{
		return readRegister<T>(S2MM_DA);
	}

	////////////////////////////////////////

	////////////////////////////////////////

	uint32_t GetMM2SByteLength()
	{
		return readRegister<uint32_t>(MM2S_LENGTH);
	}

	uint32_t GetS2MMByteLength()
	{
		return readRegister<uint32_t>(S2MM_LENGTH);
	}

	////////////////////////////////////////

private:
	////////////////////////////////////////

	void setMM2SSrcAddr(const T& addr)
	{
		writeRegister<T>(MM2S_SA, addr);
	}

	void setS2MMDestAddr(const T& addr)
	{
		writeRegister<T>(S2MM_DA, addr);
	}

	////////////////////////////////////////

	////////////////////////////////////////

	void setMM2SByteLength(const uint32_t& length)
	{
		writeRegister<uint32_t>(MM2S_LENGTH, length);
	}

	void setS2MMByteLength(const uint32_t& length)
	{
		writeRegister<uint32_t>(S2MM_LENGTH, length);
	}

	////////////////////////////////////////

public:
	class ControlRegister : public internal::Register<uint32_t>
	{
	public:
		ControlRegister(const std::string& name) :
			Register(name)
		{
			RegisterElement<bool>(&m_rs, "RS", 0);
			RegisterElement<bool>(&m_reset, "Reset", 2);
			RegisterElement<bool>(&m_keyhole, "Keyhole", 3);
			RegisterElement<bool>(&m_cyclicBDEnable, "CyclicBDEnable", 4);
			RegisterElement<bool>(&m_ioCIrqEn, "IOCIrqEn", 12);
			RegisterElement<bool>(&m_dlyIrqEn, "DlyIrqEn", 13);
			RegisterElement<bool>(&m_errIrqEn, "ErrIrqEn", 14);
			RegisterElement<uint8_t>(&m_irqThreshold, "IRQThreshold", 16, 23);
			RegisterElement<uint8_t>(&m_irqDelay, "IRQDelay", 24, 31);
		}

		void EnableInterrupts(const DMAInterrupts& intr = INTR_ALL)
		{
			setInterrupts(true, intr);
		}

		void DisableInterrupts(const DMAInterrupts& intr = INTR_ALL)
		{
			setInterrupts(false, intr);
		}

		void Start()
		{
			setRunStop(true);
		}

		void Stop()
		{
			setRunStop(false);
		}

		void DoReset()
		{
			Update();
			Reset = 1;
			Update(internal::Direction::WRITE);

			// The Reset bit will be set to 0 once the reset has been completed
			while (Reset)
				Update();
		}

	private:
		void setRunStop(bool run)
		{
			// Update the register
			Update();
			// Set/Unset the Run-Stop bit
			m_rs = run;
			// Write changes to the register
			Update(internal::Direction::WRITE);
		}

		void setInterrupts(bool enable, const DMAInterrupts& intr)
		{
			if (intr & INTR_ON_COMPLETE)
				m_ioCIrqEn = enable;
			if (intr & INTR_ON_DELAY)
				m_dlyIrqEn = enable;
			if (intr & INTR_ON_ERROR)
				m_errIrqEn = enable;

			Update(internal::Direction::WRITE);
		}

	private:
		bool m_rs              = false;
		bool m_reset           = false;
		bool m_keyhole         = false;
		bool m_cyclicBDEnable  = false;
		bool m_ioCIrqEn        = false;
		bool m_dlyIrqEn        = false;
		bool m_errIrqEn        = false;
		uint8_t m_irqThreshold = 0;
		uint8_t m_irqDelay     = 0;
	};

	class StatusRegister : public internal::Register<uint32_t>, public internal::HasInterrupt, public internal::HasStatus
	{
	public:
		StatusRegister(const std::string& name) :
			Register(name)
		{
			RegisterElement<bool>(&m_halted, "Halted", 0);
			RegisterElement<bool>(&m_idle, "Idle", 1);
			RegisterElement<bool>(&m_sgIncld, "SGIncld", 3);
			RegisterElement<bool>(&m_dmaIntErr, "DMAIntErr", 4);
			RegisterElement<bool>(&m_dmaSlvErr, "DMASlvErr", 5);
			RegisterElement<bool>(&m_dmaDecErr, "DMADecErr", 6);
			RegisterElement<bool>(&m_sgIntErr, "SGIntErr", 8);
			RegisterElement<bool>(&m_sgSlvErr, "SGSlvErr", 9);
			RegisterElement<bool>(&m_sgDecErr, "SGDecErr", 10);
			RegisterElement<bool>(&m_ioCIrq, "IOCIrq", 12);
			RegisterElement<bool>(&m_dlyIrq, "DlyIrq", 13);
			RegisterElement<bool>(&m_errIrq, "ErrIrq", 14);
			RegisterElement<uint8_t>(&m_irqThresholdSts, "IRQThresholdSts", 16, 23);
			RegisterElement<uint8_t>(&m_irqDelaySts, "IRQDelaySts", 24, 31);
		}

		void ClearInterrupts()
		{
			m_lastInterrupt = GetInterrupts();
			ResetInterrupts(INTR_ALL);
		}

		uint32_t GetInterrupts()
		{
			Update();
			uint32_t intr = 0;
			intr |= m_ioCIrq << (INTR_ON_COMPLETE >> 1);
			intr |= m_dlyIrq << (INTR_ON_DELAY >> 1);
			intr |= m_errIrq << (INTR_ON_ERROR >> 1);

			return intr;
		}

		void ResetInterrupts(const DMAInterrupts& intr)
		{
			if (intr & INTR_ON_COMPLETE)
				m_ioCIrq = 1;
			if (intr & INTR_ON_DELAY)
				m_dlyIrq = 1;
			if (intr & INTR_ON_ERROR)
				m_errIrq = 1;

			Update(internal::Direction::WRITE);
		}

	protected:
		void getStatus()
		{
			Update();
			if (!m_done && m_idle)
				m_done = true;
		}

	private:
		bool m_halted             = false;
		bool m_idle               = false;
		bool m_sgIncld            = false;
		bool m_dmaIntErr          = false;
		bool m_dmaSlvErr          = false;
		bool m_dmaDecErr          = false;
		bool m_sgIntErr           = false;
		bool m_sgSlvErr           = false;
		bool m_sgDecErr           = false;
		bool m_ioCIrq             = false;
		bool m_dlyIrq             = false;
		bool m_errIrq             = false;
		uint8_t m_irqThresholdSts = 0;
		uint8_t m_irqDelaySts     = 0;
	};

	class MM2SControlRegister : public ControlRegister
	{
	public:
		MM2SControlRegister() :
			ControlRegister("MM2S DMA Control Register")
		{
		}
	};

	class MM2SStatusRegister : public StatusRegister
	{
	public:
		MM2SStatusRegister() :
			StatusRegister("MM2S DMA Status Register")
		{
		}
	};

	class S2MMControlRegister : public ControlRegister
	{
	public:
		S2MMControlRegister() :
			ControlRegister("S2MM DMA Control Register")
		{
		}
	};

	class S2MMStatusRegister : public StatusRegister
	{
	public:
		S2MMStatusRegister() :
			StatusRegister("S2MM DMA Status Register")
		{
		}
	};

public:
	MM2SControlRegister m_mm2sCtrlReg = MM2SControlRegister();
	MM2SStatusRegister m_mm2sStatReg  = MM2SStatusRegister();
	S2MMControlRegister m_s2mmCtrlReg = S2MMControlRegister();
	S2MMStatusRegister m_s2mmStatReg  = S2MMStatusRegister();

private:
	internal::WatchDog m_watchDogMM2S;
	internal::WatchDog m_watchDogS2MM;
};
} // namespace clap