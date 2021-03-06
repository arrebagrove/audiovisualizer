#pragma once
#include "ErrorHandling.h"
#include "trace.h"
#include "BaseVisualizer.h"
#include "SpectrumData.h"
#include <vector>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::AudioVisualizer;
using namespace ABI::Windows::UI::Xaml::Controls;
using namespace ABI::Windows::UI::Xaml;
using namespace ABI::Microsoft::Graphics::Canvas;
using namespace ABI::Windows::Foundation;


namespace AudioVisualizer
{
	class SpectrumVisualizer : public RuntimeClass<
		IVisualizer,
		IBarVisualizer,
		ISpectrumVisualizer,
		ComposableBase<>>,
		BaseVisualizer<SpectrumVisualizer>
	{
		InspectableClass(RuntimeClass_AudioVisualizer_SpectrumVisualizer,BaseTrust);

		Orientation _orientation;
		std::vector<MeterBarLevel> _levels;
		Thickness _elementMargin;
		Color _unlitElement;
		UINT32 _expectedFrequencyCount;
		UINT32 _channelIndex;
		float _minAmp;
		float _maxAmp;
		SpectrumVisualizationStyle _style;
		
		Size _controlSize;

		SRWLock _lock;

		EventRegistrationToken _sourceConfigurationChangedToken;
		EventRegistrationToken _sizeChangedToken;

		bool _bIsConfigurationValid;

		void ReconfigureSpectrum(HSTRING propertyName);
	public:
		SpectrumVisualizer();
		~SpectrumVisualizer();
		// IVisualizer implementation
		STDMETHODIMP get_Source(ABI::AudioVisualizer::IVisualizationSource **ppSource)
		{
			if (ppSource == nullptr)
				return E_POINTER;
			*ppSource = _visualizationSource.Get();
			return S_OK;
		}

		STDMETHODIMP put_Source(ABI::AudioVisualizer::IVisualizationSource *pSource);

		STDMETHODIMP get_BackgroundColor(Color *pColor)
		{
			if (pColor == nullptr)
				return E_POINTER;
			*pColor = _drawingSessionBackgroundColor;
			return S_OK;
		}
		STDMETHODIMP put_BackgroundColor(Color color)
		{
			_drawingSessionBackgroundColor = color;
			return S_OK;
		}
		// IBarVisualizer implementation
		STDMETHODIMP get_Orientation(Orientation *value)
		{
			if (value == nullptr)
				return E_POINTER;
			*value = _orientation;
			return S_OK;
		}
		STDMETHODIMP put_Orientation(Orientation value)
		{
			return E_NOTIMPL;	// TODO: Support other orientation
			auto lock = _lock.LockExclusive();
			_orientation = value;
			return S_OK;
		}

		STDMETHODIMP get_ChannelIndex(UINT32 *pIndex)
		{
			if (pIndex == nullptr)
				return E_POINTER;
			*pIndex = _channelIndex;
			return S_OK;
		}
		STDMETHODIMP put_ChannelIndex(UINT32 index)
		{
			auto lock = _lock.LockExclusive();
			_channelIndex = index;
			return S_OK;
		}


		STDMETHODIMP get_Levels(UINT32 *pcElements, MeterBarLevel **ppLevels)
		{
			if (ppLevels == nullptr || pcElements == nullptr)
				return E_POINTER;
			auto lock = _lock.LockShared();
			
			*pcElements = _levels.size();
			*ppLevels = (MeterBarLevel *)CoTaskMemAlloc(sizeof(MeterBarLevel) * _levels.size());
			if (*ppLevels == nullptr)
				return E_OUTOFMEMORY;

			for (size_t i = 0; i < _levels.size(); i++)
			{
				*ppLevels[i] = _levels[i];
			}
			return S_OK;
		}

		STDMETHODIMP put_Levels(UINT32 cElements, MeterBarLevel *pLevels)
		{
			if (pLevels == nullptr)
				return E_POINTER;
			auto lock = _lock.LockExclusive();
			
			_levels.resize(cElements);
			if (cElements != 0)
			{
				_minAmp = std::numeric_limits<float>::max();
				_maxAmp = std::numeric_limits<float>::min();
				for (size_t i = 0; i < cElements; i++)
				{
					_levels[i] = pLevels[i];
					_minAmp = std::min(_minAmp, _levels[i].Level);
					_maxAmp = std::max(_maxAmp, _levels[i].Level);
				}
			}
			else
			{
				_minAmp = -100.0f;
				_maxAmp = 0.0f;
			}			
			return S_OK;
		}

		STDMETHODIMP get_RelativeElementMargin(Thickness *pElementMargin)
		{
			if (pElementMargin == nullptr)
				return E_POINTER;
			*pElementMargin = _elementMargin;
			return S_OK;
		}
		STDMETHODIMP put_RelativeElementMargin(Thickness elementMargin)
		{
			auto lock = _lock.LockExclusive();
			_elementMargin = elementMargin;
			return S_OK;
		}

		STDMETHODIMP get_UnlitElement(Color *pColor)
		{
			if (pColor == nullptr)
				return E_POINTER;
			*pColor = _unlitElement;
			return S_OK;
		}
		STDMETHODIMP put_UnlitElement(Color color)
		{
			auto lock = _lock.LockExclusive();
			_unlitElement = color;
			return S_OK;
		}

		STDMETHODIMP get_VisualizationStyle(SpectrumVisualizationStyle *pStyle)
		{
			if (pStyle == nullptr)
				return E_POINTER;
			*pStyle = _style;
			return S_OK;
		}
		STDMETHODIMP put_VisualizationStyle(SpectrumVisualizationStyle style)
		{
			auto lock = _lock.LockExclusive();
			_style = style;
			return S_OK;
		}
private:
		virtual HRESULT OnDraw(ICanvasDrawingSession *pSession, IVisualizationDataFrame *pDataFrame, IReference<TimeSpan> *pPresentationTime);

	};
}

