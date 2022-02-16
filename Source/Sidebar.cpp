/*
 // Copyright (c) 2015-2021-2022 Timothy Schoen.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include <JuceHeader.h>
#include "Pd/PdInstance.h"
#include "LookAndFeel.h"

#include "Sidebar.h"
// MARK: Inspector

struct Inspector : public PropertyPanel
{
    void paint(Graphics& g) {
        g.setColour(findColour(ComboBox::backgroundColourId).darker(0.4f));
        g.fillRect(getLocalBounds().withHeight(getTotalContentHeight()));
    }
    
    void loadParameters(ObjectParameters& params) {
        
        StringArray names = {"General", "Appearance", "Label", "Extra"};
        
        clear();
        
        auto createPanel = [](int type, String name, Value* value, Colour bg, std::vector<String>& options) -> PropertyComponent*
        {
            switch (type)
            {
                case tString:
                    return new EditableComponent<String>(name, *value, bg);
                case tFloat:
                    return new EditableComponent<float>(name, *value, bg);
                case tInt:
                    return new EditableComponent<int>(name, *value, bg);
                case tColour:
                    return new ColourComponent(name, *value, bg);
                case tBool:
                    return new BoolComponent(name, *value, bg, options);
                case tCombo:
                    return new ComboComponent(name, *value, bg, options);
                    
            }
        };
            
            
        for(int i = 0; i < 4; i++) {
            Array<PropertyComponent*> panels;
            
            int idx = 0;
            for(auto& [name, type, category, value, options] : params) {
                
                if(static_cast<int>(category) == i) {
                    auto bg = idx & 1 ? findColour(ComboBox::backgroundColourId) : findColour(ResizableWindow::backgroundColourId);
                    
                    panels.add(createPanel(type, name, value, bg, options));
                    idx++;
                }
            }
            if(!panels.isEmpty()) {
                addSection(names[i], panels);
            }
        }
        
    }

    
struct InspectorProperty : public PropertyComponent
{
    Colour bg;
    
    InspectorProperty(String propertyName, Colour background) : PropertyComponent(propertyName, 23), bg(background) {
    }
    
    void paint(Graphics& g) override {
        g.fillAll(bg);
        getLookAndFeel().drawPropertyComponentLabel(g, getWidth(), getHeight(), *this);
    }
    
    void refresh() override {};
};

struct ComboComponent : public InspectorProperty
{
    ComboComponent(String propertyName, Value& value, Colour background, std::vector<String> options) : InspectorProperty(propertyName, background), property(value)
    {
        
        for(int n = 0; n < options.size(); n++) {
            comboBox.addItem(options[n], n + 1);
        }
        
        comboBox.setName("inspector:combo");
        comboBox.setSelectedId(static_cast<int>(value.getValue()) + 1);
        
        addAndMakeVisible(comboBox);
        
        comboBox.onChange = [this]()
        {
            property = comboBox.getSelectedId() - 1;
            callback(row);
        };
    }
    
    void resized() override
    {
        comboBox.setBounds(getLocalBounds().removeFromLeft(getWidth() / 2));
    }
    
private:
    
    Value& property;
    std::function<void(int)> callback;
    int row;
    ComboBox comboBox;
    
};

struct BoolComponent : public InspectorProperty
{
    BoolComponent(String propertyName, Value& value, Colour background, std::vector<String> options) : InspectorProperty(propertyName, background)
    {
        toggleButton.setClickingTogglesState(true);
        
        toggleButton.setConnectedEdges(12);
        
        toggleButton.getToggleStateValue().referTo(value);
        toggleButton.setButtonText(static_cast<bool>(value.getValue()) ? options[1] : options[0]);
        
        toggleButton.setName("inspector:toggle");
        
        addAndMakeVisible(toggleButton);
        
        toggleButton.onClick = [this, value, options]() mutable
        {
            toggleButton.setButtonText(toggleButton.getToggleState() ? options[1] : options[0]);
        };
    }
    
    void resized() override
    {
        toggleButton.setBounds(getLocalBounds().removeFromRight(getWidth() / 2));
    }
    
private:
    TextButton toggleButton;
};

struct ColourComponent : public InspectorProperty, public ChangeListener
{
    
    ColourComponent(String propertyName, Value& value, Colour background) : InspectorProperty(propertyName, background), currentColour(value)
    {
        String strValue = currentColour.toString();
        if (strValue.length() > 2)
        {
            button.setButtonText(String("#") + strValue.substring(2));
        }
        button.setConnectedEdges(12);
        button.setColour(ComboBox::outlineColourId, Colours::transparentBlack);
        
        addAndMakeVisible(button);
        updateColour();
        
        button.onClick = [this]()
        {
            std::unique_ptr<ColourSelector> colourSelector = std::make_unique<ColourSelector>(ColourSelector::showColourAtTop | ColourSelector::showSliders | ColourSelector::showColourspace);
            colourSelector->setName("background");
            colourSelector->setCurrentColour(findColour(TextButton::buttonColourId));
            colourSelector->addChangeListener(this);
            colourSelector->setSize(300, 400);
            colourSelector->setColour(ColourSelector::backgroundColourId, findColour(ComboBox::backgroundColourId));
            
            colourSelector->setCurrentColour(Colour::fromString(currentColour.toString()));
            
            CallOutBox::launchAsynchronously(std::move(colourSelector), button.getScreenBounds(), nullptr);
        };
    }
    
    void updateColour()
    {
        auto colour = Colour::fromString(currentColour.toString());
        
        button.setColour(TextButton::buttonColourId, colour);
        button.setColour(TextButton::buttonOnColourId, colour.brighter());
        
        auto textColour = colour.getPerceivedBrightness() > 0.5 ? Colours::black : Colours::white;
        
        // make sure text is readable
        button.setColour(TextButton::textColourOffId, textColour);
        button.setColour(TextButton::textColourOnId, textColour);
        
        button.setButtonText(String("#") + currentColour.toString().substring(2));
    }
    
    void changeListenerCallback(ChangeBroadcaster* source) override
    {
        auto* cs = dynamic_cast<ColourSelector*>(source);
        
        auto colour = cs->getCurrentColour();
        currentColour = colour.toString();
        
        updateColour();
    }
    
    ~ColourComponent() override = default;
    
    void resized() override
    {
        button.setBounds(getLocalBounds().removeFromRight(getWidth() / 2));
    }
    
private:
    TextButton button;
    Value& currentColour;
};

template <typename T>
struct EditableComponent : public InspectorProperty
{
    Label label;
    Value& property;
    float downValue;
    
    EditableComponent(String propertyName, Value& value, Colour background) : InspectorProperty(propertyName, background), property(value)
    {
        addAndMakeVisible(label);
        label.setEditable(false, true);
        label.getTextValue().referTo(property);
        label.addMouseListener(this, true);
        
        label.onEditorShow = [this](){
            auto* editor = label.getCurrentTextEditor();
            
            if constexpr (std::is_floating_point<T>::value)
            {
                editor->setInputRestrictions(0, "0123456789.-");
            }
            else if constexpr (std::is_integral<T>::value)
            {
                editor->setInputRestrictions(0, "0123456789-");
            }
        };
    }
    
    void mouseDown(const MouseEvent& event) override
    {
        if constexpr (!std::is_arithmetic<T>::value) return;
        
        downValue = label.getText().getFloatValue();
    }
    
    void mouseDrag(const MouseEvent& e) override {
        
        if constexpr (!std::is_arithmetic<T>::value) return;
        
        auto const inc = static_cast<float>(-e.getDistanceFromDragStartY()) * 0.5f;
        if (std::abs(inc) < 1.0f) return;
        
        // Logic for dragging, where the x position decides the precision
        auto currentValue = label.getText();
        if (!currentValue.containsChar('.')) currentValue += '.';
        if (currentValue.getCharPointer()[0] == '-') currentValue = currentValue.substring(1);
        currentValue += "00000";
        
        // Get position of all numbers
        Array<int> glyphs;
        Array<float> xOffsets;
        label.getFont().getGlyphPositions(currentValue, glyphs, xOffsets);
        
        // Find the number closest to the mouse down
        auto position = static_cast<float>(e.getMouseDownX() - 4);
        auto precision = static_cast<int>(std::lower_bound(xOffsets.begin(), xOffsets.end(), position) - xOffsets.begin());
        precision -= currentValue.indexOfChar('.');
        
        // I case of integer dragging
        if (precision <= 0)
        {
            precision = 0;
        }
        else
        {
            // Offset for the decimal point character
            precision -= 1;
        }
        
        if constexpr (std::is_integral<T>::value)
        {
            precision = 0;
        }
        
        // Calculate increment multiplier
        float multiplier = powf(10.0f, static_cast<float>(-precision));
        
        // Calculate new value as string
        auto newValue = String(downValue + inc * multiplier, precision);
        
        if (precision == 0) newValue = newValue.upToFirstOccurrenceOf(".", true, false);
        
        if constexpr (std::is_integral<T>::value)
        {
            newValue = newValue.upToFirstOccurrenceOf(".", false, false);
        }
        
        label.setText(newValue, sendNotification);
        
        
    }
    
    void resized() override {
        label.setBounds(getLocalBounds().removeFromRight(getWidth() / 2));
    }
};

    
};

// MARK: Console


struct ConsoleComponent : public Component, public ComponentListener
{
    std::array<TextButton, 5>& buttons;
    Viewport& viewport;
    
    std::vector<std::pair<String, int>> messages;
    std::vector<std::pair<String, int>> history;
    
    ConsoleComponent(std::array<TextButton, 5>& b, Viewport& v) : buttons(b), viewport(v)
    {
        update();
    }
    
    void componentMovedOrResized(Component& component, bool wasMoved, bool wasResized) override
    {
        setSize(viewport.getWidth(), getHeight());
        repaint();
    }
    
public:
    
    void update()
    {
        repaint();
        setSize(viewport.getWidth(), std::max<int>(getTotalHeight(), viewport.getHeight()));
        
        if (buttons[4].getToggleState())
        {
            viewport.setViewPositionProportionately(0.0f, 1.0f);
        }
    }
    
    
    void clear()
    {
        messages.clear();
    }
    
    
    void mouseDown(const MouseEvent& e) override {
        // TODO: implement selecting and copying comments
    }
    
    void paint(Graphics& g) override
    {
        auto font = Font(Font::getDefaultSansSerifFontName(), 13, 0);
        g.setFont(font);
        g.fillAll(findColour(ComboBox::backgroundColourId));
        
        
        int totalHeight = 0;
        
        int numEmpty = 0;
        
        bool showMessages = buttons[2].getToggleState();
        bool showErrors = buttons[3].getToggleState();
        
        for (int row = 0; row < jmax(32, static_cast<int>(messages.size())); row++)
        {
            int height = 24;
            int numLines = 1;
            
            if (isPositiveAndBelow(row, messages.size()))
            {
                auto& e = messages[row];
                
                if((e.second == 1 && !showMessages) || (e.second == 2 && !showErrors)) continue;
                
                Array<int> glyphs;
                Array<float> xOffsets;
                font.getGlyphPositions(e.first, glyphs, xOffsets);
                
                for (int i = 0; i < xOffsets.size(); i++)
                {
                    if ((xOffsets[i] + 10) >= static_cast<float>(getWidth()))
                    {
                        height += 22;
                        
                        for (int j = i + 1; j < xOffsets.size(); j++)
                        {
                            xOffsets.getReference(j) -= xOffsets[i];
                        }
                        numLines++;
                    }
                }
            }
            
            const Rectangle<int> r(0, totalHeight, getWidth(), height);
            
            if (row % 2 || row == selectedItem)
            {
                g.setColour(selectedItem == row ? findColour(Slider::thumbColourId) : findColour(ResizableWindow::backgroundColourId));
                
                g.fillRect(r);
            }
            
            if (isPositiveAndBelow(row, messages.size()))
            {
                const auto& e = messages[row];
                
                g.setColour(selectedItem == row ? Colours::white : colourWithType(e.second));
                g.drawFittedText(e.first, r.reduced(4, 0), Justification::centredLeft, numLines, 1.0f);
            }
            else
            {
                numEmpty++;
            }
            
            totalHeight += height;
        }
        
        totalHeight -= numEmpty * 24;
    }
    
    // Get total height of messages, also taking multi-line messages into account
    // TODO: pre-calculate the number of lines in messages!!
    int getTotalHeight()
    {
        auto font = Font(Font::getDefaultSansSerifFontName(), 13, 0);
        int totalHeight = 0;
        
        int numEmpty = 0;
        
        for (int row = 0; row < jmax(32, static_cast<int>(messages.size())); row++)
        {
            int height = 24;
            int numLines = 1;
            
            if (isPositiveAndBelow(row, messages.size()))
            {
                auto& e = messages[row];
                
                Array<int> glyphs;
                Array<float> xOffsets;
                font.getGlyphPositions(e.first, glyphs, xOffsets);
                
                for (int i = 0; i < xOffsets.size(); i++)
                {
                    if ((xOffsets[i] + 10) >= static_cast<float>(getWidth()))
                    {
                        height += 22;
                        
                        for (int j = i + 1; j < xOffsets.size(); j++)
                        {
                            xOffsets.getReference(j) -= xOffsets[i];
                        }
                        numLines++;
                    }
                }
            }
            
            if (!isPositiveAndBelow(row, messages.size()))
            {
                numEmpty++;
            }
            
            totalHeight += height;
        }
        
        totalHeight -= numEmpty * 24;
        
        return totalHeight;
    }
    
    void resized() override
    {
        update();
    }
    
private:
    static Colour colourWithType(int type)
    {
        auto c = Colours::red;
        
        if (type == 0)
        {
            c = Colours::white;
        }
        else if (type == 1)
        {
            c = Colours::orange;
        }
        else if (type == 2)
        {
            c = Colours::red;
        }
        
        return c;
    }
    
    static void removeMessagesIfRequired(std::deque<std::pair<String, int>>& messages)
    {
        const int maximum = 2048;
        const int removed = 64;
        
        int size = static_cast<int>(messages.size());
        
        if (size >= maximum)
        {
            const int n = nextPowerOfTwo(size - maximum + removed);
            
            jassert(n < size);
            
            messages.erase(messages.cbegin(), messages.cbegin() + n);
        }
    }
    
    template <class T>
    static void parseMessages(T& m, bool showMessages, bool showErrors)
    {
        if (!showMessages || !showErrors)
        {
            auto f = [showMessages, showErrors](const std::pair<String, bool>& e)
            {
                bool t = e.second;
                
                if ((t && !showMessages) || (!t && !showErrors))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            };
            
            m.erase(std::remove_if(m.begin(), m.end(), f), m.end());
        }
    }
    
    int selectedItem = -1;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConsoleComponent)
};

struct Console : public Component
{
    Console()
    {
        // Viewport takes ownership
        console = new ConsoleComponent(buttons, viewport);
        
        addComponentListener(console);
        
        viewport.setViewedComponent(console);
        viewport.setScrollBarsShown(true, false);
        console->setVisible(true);
        
        addAndMakeVisible(viewport);
        
        std::vector<String> tooltips = {"Clear logs", "Restore logs", "Show errors", "Show messages", "Enable autoscroll"};
        
        std::vector<std::function<void()>> callbacks = {
            [this]() { /*console->clear();*/ },
            [this]() { /* console->restore(); */ },
            [this]()
            {
                repaint();
                /*
                 if (buttons[2].getState())
                 console->restore();
                 else
                 console->parse(); */
            },
            [this]()
            {
                repaint();
                /*
                 if (buttons[3].getState())
                 console->restore();
                 else
                 console->parse(); */
            },
            [this]()
            {
                if (buttons[4].getState())
                {
                    console->update();
                }
            },
            
        };
        
        int i = 0;
        for (auto& button : buttons)
        {
            button.setName("statusbar:console");
            button.setConnectedEdges(12);
            addAndMakeVisible(button);
            
            button.onClick = callbacks[i];
            button.setTooltip(tooltips[i]);
            
            i++;
        }
        
        buttons[2].setClickingTogglesState(true);
        buttons[3].setClickingTogglesState(true);
        buttons[4].setClickingTogglesState(true);
        
        buttons[2].setToggleState(true, sendNotification);
        buttons[3].setToggleState(true, sendNotification);
        buttons[4].setToggleState(true, sendNotification);
        
        resized();
    }
    
    ~Console() override
    {
        removeComponentListener(console);
    }
    
    
    
    void resized() override
    {
        FlexBox fb;
        fb.flexWrap = FlexBox::Wrap::noWrap;
        fb.justifyContent = FlexBox::JustifyContent::flexStart;
        fb.alignContent = FlexBox::AlignContent::flexStart;
        fb.flexDirection = FlexBox::Direction::row;
        
        for (auto& b : buttons)
        {
            auto item = FlexItem(b).withMinWidth(8.0f).withMinHeight(8.0f).withMaxHeight(27);
            item.flexGrow = 1.0f;
            item.flexShrink = 1.0f;
            fb.items.add(item);
        }
        
        auto bounds = getLocalBounds().toFloat();
        
        fb.performLayout(bounds.removeFromBottom(27));
        viewport.setBounds(bounds.toNearestInt());
    }
    
    ConsoleComponent* console;
    Viewport viewport;
    
    std::array<TextButton, 5> buttons = {TextButton(Icons::Clear), TextButton(Icons::Restore), TextButton(Icons::Error), TextButton(Icons::Message), TextButton(Icons::AutoScroll)};
};


Sidebar::Sidebar(pd::Instance* instance) : pd(instance) {
    // Can't use RAII because unique pointer won't compile with forward declarations
    console = new Console;
    inspector = new Inspector;
    
    addAndMakeVisible(console);
    addAndMakeVisible(inspector);
    

    setBounds(getParentWidth() - lastWidth, 40, lastWidth, getParentHeight() - 65);
}

Sidebar::~Sidebar() {
    delete console;
    delete inspector;
}

void Sidebar::paint(Graphics& g) {
    
    int sWidth = sidebarHidden ? dragbarWidth : std::max(dragbarWidth, getWidth());
        
    // Sidebar
    g.setColour(findColour(ComboBox::backgroundColourId).darker(0.1));
    g.fillRect(getWidth() - sWidth, 0, sWidth, getHeight());
    
}

void Sidebar::paintOverChildren(Graphics& g) {
    
    int sWidth = sidebarHidden ? dragbarWidth : std::max(dragbarWidth, getWidth());
    
    // Draggable bar
    g.setColour(findColour(ComboBox::backgroundColourId));
    g.fillRect(getWidth() - sWidth, 0, dragbarWidth + 1, getHeight());
}

void Sidebar::resized() {
    
    auto bounds = getLocalBounds();
    bounds.removeFromLeft(dragbarWidth);
    
    console->setBounds(bounds);
    inspector->setBounds(bounds);
}

void Sidebar::mouseDown(const MouseEvent& e)
{
    Rectangle<int> dragBar(0, dragbarWidth, getWidth(), getHeight());
    if (dragBar.contains(e.getPosition()) && !sidebarHidden)
    {
        draggingSidebar = true;
        dragStartWidth = getWidth();
    }
    else
    {
        draggingSidebar = false;
    }
}

void Sidebar::mouseDrag(const MouseEvent& e)
{
    if (draggingSidebar)
    {
        int newWidth = dragStartWidth - e.getDistanceFromDragStartX();
        
        setBounds(getParentWidth() - newWidth, getY(), newWidth, getHeight());
        getParentComponent()->resized();
    }
}

void Sidebar::mouseUp(const MouseEvent& e)
{
    if (draggingSidebar)
    {
        //getCurrentCanvas()->checkBounds(); fix this
        draggingSidebar = false;
    }
}

void Sidebar::showSidebar(bool show) {
    sidebarHidden = !show;
    
    if(!show)
    {
        lastWidth = getWidth();
        int newWidth = dragbarWidth;
        setBounds(getParentWidth() - newWidth, getY(), newWidth, getHeight());
    }
    else {
        int newWidth = lastWidth;
        setBounds(getParentWidth() - newWidth, getY(), newWidth, getHeight());
    }
}

void Sidebar::showParameters(ObjectParameters& params) {
    lastParameters = params;
    inspector->loadParameters(params);
    
    inspector->setVisible(true);
    console->setVisible(false);
}


void Sidebar::showParameters() {
    inspector->loadParameters(lastParameters);
    inspector->setVisible(true);
    console->setVisible(false);
}
void Sidebar::hideParameters() {
    inspector->setVisible(false);
    console->setVisible(true);
}

bool Sidebar::isShowingConsole() const noexcept {
    return console->isVisible();
}


void Sidebar::updateConsole()
{
    console->console->messages = pd->consoleMessages;
    console->repaint();
}
