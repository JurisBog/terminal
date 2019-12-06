﻿// Copyright (c) Microsoft Corporation
// Licensed under the MIT license.

#include "pch.h"
#include "SearchBoxControl.h"
#include "SearchBoxControl.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Core;

namespace winrt::Microsoft::Terminal::TerminalControl::implementation
{
    // Constructor
    SearchBoxControl::SearchBoxControl() :
        _goForward(false),
        _isCaseSensitive(false)
    {
        InitializeComponent();

        this->CharacterReceived({ this, &SearchBoxControl::_CharacterHandler });

        _goForwardButton = this->FindName(L"SetGoForwardButton").try_as<Controls::Primitives::ToggleButton>();
        _goBackwardButton = this->FindName(L"SetGoBackwardButton").try_as<Controls::Primitives::ToggleButton>();
        _textBox = this->FindName(L"TextBox").try_as<Controls::TextBox>();

        auto closeButton = this->FindName(L"CloseButton").try_as<Controls::Button>();
        auto caseButton = this->FindName(L"CaseSensitivityButton").try_as<Controls::Primitives::ToggleButton>();

        if (closeButton)
            _focusableElements.insert(closeButton);
        if (_textBox)
            _focusableElements.insert(_textBox);
        if (caseButton)
            _focusableElements.insert(caseButton);
        if (_goForwardButton)
            _focusableElements.insert(_goForwardButton);
        if (_goBackwardButton)
            _focusableElements.insert(_goBackwardButton);
    }

    // Method Description:
    // - Getter for _goForward
    // Arguments:
    // - <none>
    // Return Value:
    // - bool: the value of _goForward
    bool SearchBoxControl::GetGoForward()
    {
        return _goForward;
    }

    // Method Description:
    // - Getter for _isCaseSensitive
    // Arguments:
    // - <none>
    // Return Value:
    // - bool: the value of _isCaseSensitive
    bool SearchBoxControl::GetIsCaseSensitive()
    {
        return _isCaseSensitive;
    }

    // Method Description:
    // - Handler for pressing Enter on TextBox, trigger
    //   text search
    // Arguments:
    // - sender: not used
    // - e: event data
    // Return Value:
    // - <none>
    void SearchBoxControl::QuerySubmitted(winrt::Windows::Foundation::IInspectable const& /*sender*/, Input::KeyRoutedEventArgs const& e)
    {
        if (e.OriginalKey() == winrt::Windows::System::VirtualKey::Enter)
        {
            auto const state = CoreWindow::GetForCurrentThread().GetKeyState(winrt::Windows::System::VirtualKey::Shift);
            if (WI_IsFlagSet(state, CoreVirtualKeyStates::Down) || state == (CoreVirtualKeyStates::Locked | CoreVirtualKeyStates::Down))
            {
                // We do not want the direction flag to change permanately
                _goForward = !_goForward;
                _searchEventHandler(*this, _textBox.Text());
                _goForward = !_goForward;
            }
            else
            {
                _searchEventHandler(*this, _textBox.Text());
            }
        }
    }

    // Method Description:
    // - Handler for pressing Enter on TextBox, trigger
    //   text search
    // Arguments:
    // - <none>
    // Return Value:
    // - <none>
    void SearchBoxControl::SetFocusOnTextbox()
    {
        if (_textBox)
        {
            Input::FocusManager::TryFocusAsync(_textBox, FocusState::Keyboard);
            _textBox.SelectAll();
        }
    }

    // Method Description:
    // - Check if the current focus is on any element within the
    //   search box
    // Arguments:
    // - <none>
    // Return Value:
    // - bool: whether the current focus is on the search box
    bool SearchBoxControl::ContainsFocus()
    {
        auto focusedElement = Input::FocusManager::GetFocusedElement(this->XamlRoot());
        if (_focusableElements.count(focusedElement) > 0)
        {
            return true;
        }

        return false;
    }

    // Method Description:
    // - Handler for clicking the GoBackward button. This change the value of _goForward,
    //   mark GoBackward button as checked and ensure GoForward button
    //   is not checked
    // Arguments:
    // - sender: not used
    // - e: not used
    // Return Value:
    // - <none>
    void SearchBoxControl::_GoBackwardClicked(winrt::Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        _goForward = false;
        _goBackwardButton.IsChecked(true);
        if (_goForwardButton.IsChecked())
        {
            _goForwardButton.IsChecked(false);
        }

        // kick off search
        _searchEventHandler(*this, _textBox.Text());
    }

    // Method Description:
    // - Handler for clicking the GoForward button. This change the value of _goForward,
    //   mark GoForward button as checked and ensure GoBackward button
    //   is not checked
    // Arguments:
    // - sender: not used
    // - e: not used
    // Return Value:
    // - <none>
    void SearchBoxControl::_GoForwardClicked(winrt::Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        _goForward = true;
        _goForwardButton.IsChecked(true);
        if (_goBackwardButton.IsChecked())
        {
            _goBackwardButton.IsChecked(false);
        }

        // kick off search
        _searchEventHandler(*this, _textBox.Text());
    }

    // Method Description:
    // - Handler for clicking the Case Sensitivity button. This changes
    //   _isCaseSensitive value
    // Arguments:
    // - sender: not used
    // - e: not used
    // Return Value:
    // - <none>
    void SearchBoxControl::_CaseSensitivityClicked(winrt::Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        _isCaseSensitive = !_isCaseSensitive;
    }

    // Method Description:
    // - Handler for clicking the close button. This destructs the
    //   search box object in TermControl
    // Arguments:
    // - sender: not used
    // - e: event data
    // Return Value:
    // - <none>
    void SearchBoxControl::_CloseClick(winrt::Windows::Foundation::IInspectable const& /*sender*/, RoutedEventArgs const& e)
    {
        _CloseButtonClickedHanlder(*this, e);
    }

    // Method Description:
    // - Handler for pressing Enter when focusing on the checkbox.
    //   This is implented because XAML checkbox does not support
    //   Enter
    // Arguments:
    // - sender: the XAML element responding to the pointer input
    // - e: event data
    // Return Value:
    // - <none>
    void SearchBoxControl::_CheckboxKeyDown(winrt::Windows::Foundation::IInspectable const& sender, Input::KeyRoutedEventArgs const& e)
    {
        // Checkbox does not got checked when focused and pressing Enter, we
        // manually listen to Enter and check here
        if (e.OriginalKey() == winrt::Windows::System::VirtualKey::Enter)
        {
            auto checkbox = sender.try_as<Controls::CheckBox>();
            if (checkbox.IsChecked().GetBoolean() == true)
            {
                checkbox.IsChecked(false);
            }
            else
            {
                checkbox.IsChecked(true);
            }
            e.Handled(true);
        }
    }

    // Method Description:
    // - To avoid Characters input bubbling up to terminal, we implement this handler here,
    //   simply mark the key input as handled
    // Arguments:
    // - sender: not used
    // - e: event data
    // Return Value:
    // - <none>
    void SearchBoxControl::_CharacterHandler(winrt::Windows::Foundation::IInspectable const& /*sender*/, Input::CharacterReceivedRoutedEventArgs const& e)
    {
        e.Handled(true);
    }

    // Events proxies
    DEFINE_EVENT_WITH_TYPED_EVENT_HANDLER(SearchBoxControl, Search, _searchEventHandler, TerminalControl::SearchBoxControl, winrt::hstring);
    DEFINE_EVENT_WITH_TYPED_EVENT_HANDLER(SearchBoxControl, CloseButtonClicked, _CloseButtonClickedHanlder, TerminalControl::SearchBoxControl, Windows::UI::Xaml::RoutedEventArgs);
}