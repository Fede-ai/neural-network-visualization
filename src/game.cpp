#include "game.h"
#include <iostream>

void Game::play()
{
    initAi();
    ai->setRandomValues();

    for (int i = 0; i < aiSize.size(); i++)
	{
		maxSize = std::max(maxSize, aiSize[i]);
	}
	nnHeight = maxSize * 100 + (maxSize + 1) * 80;

	neuron.setRadius(50);
	neuron.setOrigin(50, 50);
	neuron.setOutlineThickness(5);
	neuron.setOutlineColor(sf::Color::Black);

	inputs.clear();
	inputs.push_back(30);
	inputs.push_back(-30);
	ai->calculateOutput(inputs);

    sliderBg.setSize(sf::Vector2f(260, 50));
    sliderBg.setFillColor(sf::Color::White);
    sliderBg.setOutlineColor(sf::Color::Black);
    sliderBg.setOutlineThickness(6);
    slider.setFillColor(sf::Color(150, 150, 150)); 

	weight.setPrimitiveType(sf::Lines);
	weight.resize(2);

	font.loadFromFile("./resources/font.ttf");
	text.setFillColor(sf::Color::Black);
	text.setFont(font);
	text.setCharacterSize(60);
	text.setScale(0.25, 0.25);
	lastMousePos = sf::Mouse::getPosition();

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	nn.setSmooth(true);
	nn.create(1600, 1080, settings);
	setView();

	window.create(sf::VideoMode(sf::VideoMode::getDesktopMode().width * 2/3, sf::VideoMode::getDesktopMode().width * 3/8), "neural network visualization", sf::Style::Default, settings);
    window.setFramerateLimit(60);
    window.setView(sf::View(sf::Vector2f(1920/2, 1080/2), sf::Vector2f(1920, 1080)));
    window.requestFocus();

    bool isFullscreen = false;
    bool canFullscreen = false;

	while (window.isOpen())
	{
        pressed = false;
		//handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
            switch (event.type)
            {
			case sf::Event::Closed:
				window.close();
                break;
			case sf::Event::MouseWheelScrolled:
            {
                float scroll = event.mouseWheelScroll.delta;
                if (window.mapPixelToCoords(sf::Mouse::getPosition(window)).x > 320)
		        {
		    	    sf::View view(nn.getView());
		    	    if (scroll > 0)
		    	    	view.zoom(1 - 0.1 * scroll);
		    	    else
		    		view.zoom(1 / (1 + 0.1*scroll));
		    	    nn.setView(view);			
		        }
		        else
		        {
			        sliderScroll = std::min(sliderScroll + scroll * 60, 0.f);
		        }
                break;                
            }
            case sf::Event::KeyPressed:
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) && sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
				{
					if (canFullscreen)
					{
						if (isFullscreen)
						{
                            sf::View view(window.getView());
							window.create(sf::VideoMode(sf::VideoMode::getDesktopMode().width * 2/3, sf::VideoMode::getDesktopMode().width * 3/8), "simple", sf::Style::Default, settings);
							window.setFramerateLimit(60);
							window.setView(view);
						}
						else
						{	
                            sf::View view(window.getView());
							window.create(sf::VideoMode(), "simple", sf::Style::Fullscreen, settings);
							window.setFramerateLimit(60);
							window.setView(view);
						}
						isFullscreen = !isFullscreen;
					}
					canFullscreen = false;
				}
				else
				{
					canFullscreen = true;
				}
                break;
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Button::Left)
                    pressed = true;
                break;
            }
		}

		window.clear(sf::Color(150, 150, 150));

        update();
        draw();
        drawSliders();

        window.display();
	}
}

void Game::update()
{
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
	if (pressed)
    {
        if (mousePos.x > 320)
        {
		    isMooving = true;
        }
        else if (mousePos.x > 30 && mousePos.x < 290)
        {
            float i = (mousePos.y - (30 + sliderScroll)) / 80.f;
            if ((i - (int)i) < 0.6 && i > 0)
                editingValue = i;
        }  
    }

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && isMooving)
    {
        sf::View newView(nn.getView());
		sf::Vector2f diff = sf::Vector2f(lastMousePos - sf::Mouse::getPosition());
		diff = sf::Vector2f(diff.x *nn.getView().getSize().x/window.getSize().x*1.2, diff.y *nn.getView().getSize().y/window.getSize().y);
        newView.move(diff);
        nn.setView(newView);
    }
	else
	{
		isMooving = false;
	}

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && editingValue != -1)
    {
        //check if the edited value is an input
        if (editingValue <= 1)
        {
            inputs[editingValue] = std::min(std::max((mousePos.x - 160) * 50 / 130.f, -50.f), 50.f);
        }
        else
        {
            int potentialValue = 2;
            //check if the edited value is a weight
            for (int layer = 1; layer < aiSize.size(); layer++)
	        {
        		for (int neurBefore = 0; neurBefore < aiSize[layer - 1]; neurBefore++)
        		{
        			for (int neur = 0; neur < aiSize[layer]; neur++)
        			{
        				if (potentialValue == editingValue)
                            ai->setWeight(layer - 1, neur, neurBefore, std::min(std::max((mousePos.x - 160) * 1 / 130.f, -1.f), 1.f));

                        potentialValue++;       
        			}
        		}
        	}
            //check if the edited value is a bias
            for (int layer = 1; layer < aiSize.size(); layer++)
	        {
	        	for (int neur = 0; neur < aiSize[layer]; neur++)
	        	{
                    if (potentialValue == editingValue)
                        ai->setBias(layer - 1, neur, std::min(std::max((mousePos.x - 160) * 5 / 130.f, -5.f), 5.f));

                    potentialValue++; 
	        	}
	        }
        }
        ai->calculateOutput(inputs);
    }
    else if (editingValue != -1)
    {
        editingValue = -1;
        editedValue = true;
    } 

    if (editedValue)
    {
        ai->calculateOutput(inputs);
        editedValue = false;
    }

	lastMousePos = sf::Mouse::getPosition();
}

void Game::draw()
{
    nn.clear(sf::Color(200, 200, 200));
	
	//draw weights
	for (int layer = 1; layer < aiSize.size(); layer++)
	{
		float space = (nnHeight - 100 * aiSize[layer]) / (aiSize[layer] + 1);
		float spaceBefore = (nnHeight - 100 * aiSize[layer - 1]) / (aiSize[layer - 1] + 1);
		for (int neur = 0; neur < aiSize[layer]; neur++)
		{
			weight[1].position = sf::Vector2f(120 + 300 * layer, (space + 100) * neur + space + 50);
			for (int neurBefore = 0; neurBefore < aiSize[layer - 1]; neurBefore++)
			{
				weight[0].position = sf::Vector2f(120 + 300 * (layer - 1), (spaceBefore + 100) * neurBefore + spaceBefore + 50);
				nn.draw(weight);
			}
		}
	}

	//draw neuron
	for (int layer = 0; layer < aiSize.size(); layer++)
	{
		float space = (nnHeight - 100 * aiSize[layer]) / (aiSize[layer] + 1);
		for (int neur = 0; neur < aiSize[layer]; neur++)
		{
			if (layer > 0)
				neuron.setFillColor(sf::Color((1 - ai->getNeuron(layer - 1, neur)) * 255, ai->getNeuron(layer - 1, neur) * 255, 0));
			else
				neuron.setFillColor(sf::Color::White);
            
			neuron.setPosition(120 + 300 * layer, (space + 100) * neur + space + 50);
			nn.draw(neuron);
		}
	}

	//draw text
	for (int layer = 1; layer < aiSize.size(); layer++)
	{
		float space = (nnHeight - 100 * aiSize[layer]) / (aiSize[layer] + 1);
		float spaceBefore = (nnHeight - 100 * aiSize[layer - 1]) / (aiSize[layer - 1] + 1);
		for (int neur = 0; neur < aiSize[layer]; neur++)
		{
			for (int neurBefore = 0; neurBefore < aiSize[layer - 1]; neurBefore++)
			{
				//draw weight text
				float xDiff = 300;
				float yDiff = (space + 100) * neur + space + 50 - ((spaceBefore + 100) * neurBefore + spaceBefore + 50);
				std::string weight(std::to_string((round(ai->getWeight(layer - 1, neur, neurBefore)*1000))/1000)); 
				weight.erase(weight.end()-1);
                weight.erase(weight.end()-2);
				text.setString(weight);
				text.setOrigin(text.getGlobalBounds().width*2, text.getGlobalBounds().height*2);
				text.setRotation(atan(yDiff/xDiff) * 180 / 3.1415);
				text.setPosition(120 + 300 * layer - 150, ((space + 100) * neur + space + 50 + (spaceBefore + 100) * neurBefore + spaceBefore + 50) /2 - 5);
				nn.draw(text);
				text.setRotation(0);
			}
			//draw neuron text
			text.setString(std::to_string(ai->getNeuron(layer - 1, neur)));
			text.setOrigin(text.getGlobalBounds().width*2, text.getGlobalBounds().height*2);
			text.setPosition(120 + 300 * layer, (space + 100) * neur + space + 50 - 5);
			nn.draw(text);

			//draw bias text
			std::string bias(std::to_string((round(ai->getBias(layer - 1, neur)*1000))/1000)); 
			bias.erase(bias.end()-1);
            bias.erase(bias.end()-2);
			text.setString("bias: " + bias);
			text.setOrigin(text.getGlobalBounds().width*2, text.getGlobalBounds().height*2);
			text.setPosition(120 + 300 * layer, (space + 100) * neur + space - 15);
			nn.draw(text);
		}
	}

	//draw input layer neuron text
	for (int input = 0; input < aiSize[0]; input++)
	{
		float space = (nnHeight - 100 * aiSize[0]) / (aiSize[0] + 1);
        std::string value(std::to_string(inputs[input]));
        value.erase(value.end()-1);
        value.erase(value.end()-2);
        value.erase(value.end()-3);
		text.setString(value);
		text.setOrigin(text.getGlobalBounds().width*2, text.getGlobalBounds().height*2);
		text.setPosition(120, (space + 100) * input + space + 50 - 5);
		nn.draw(text);
	}
	
	nn.display();
	sf::RectangleShape nnImage;
	nnImage.setSize(sf::Vector2f(1600, 1080));
	nnImage.setPosition(sf::Vector2f(320, 0));
	nnImage.setTexture(&nn.getTexture());

	window.draw(nnImage);
}

void Game::drawSliders()
{   
    text.setCharacterSize(20);
	text.setScale(1, 1);

    int i = 0;
    //draw input slider
    for (int input = 0; input < aiSize[0]; input++)
	{
		sliderBg.setPosition(30, 30 + i * 80 + sliderScroll);
		window.draw(sliderBg);

        slider.setPosition(160, 30 + i * 80 + sliderScroll);
        slider.setSize(sf::Vector2f(130.f / 50 * inputs[input], 50));
        window.draw(slider);        
        
        std::string str("input(" + std::to_string(input + 1) + "): ");
        std::string value(std::to_string((int)round(inputs[input])));
        text.setString(str + value);
        text.setOrigin(text.getGlobalBounds().width/2, text.getGlobalBounds().height/2);
        text.setPosition(160, 30 + i * 80 + sliderScroll + 22);
        window.draw(text);
        i++;
	}
    //draw weight slider
    for (int layer = 1; layer < aiSize.size(); layer++)
	{
		for (int neurBefore = 0; neurBefore < aiSize[layer - 1]; neurBefore++)
		{
			for (int neur = 0; neur < aiSize[layer]; neur++)
			{
				sliderBg.setPosition(30, 30 + i * 80 + sliderScroll);
				window.draw(sliderBg);

                slider.setPosition(160, 30 + i * 80 + sliderScroll);
                slider.setSize(sf::Vector2f(130.f / 1 * ai->getWeight(layer - 1, neur, neurBefore), 50));
                window.draw(slider);  

                std::string str("weight(" + std::to_string(layer) + ", " + std::to_string(neurBefore+1) + ", " + std::to_string(neur+1) + "): ");
                std::string value(std::to_string((round(ai->getWeight(layer - 1, neur, neurBefore)*1000))/1000)); 
				value.erase(value.end()-1);
                value.erase(value.end()-2);
                text.setString(str + value);
                text.setOrigin(text.getGlobalBounds().width/2, text.getGlobalBounds().height/2);
                text.setPosition(160, 30 + i * 80 + sliderScroll + 22);
                window.draw(text);
                i++;
			}
		}
	}
    //draw bias slider
    for (int layer = 1; layer < aiSize.size(); layer++)
	{
		for (int neur = 0; neur < aiSize[layer]; neur++)
		{
			sliderBg.setPosition(30, 30 + i * 80 + sliderScroll);
			window.draw(sliderBg);

            slider.setPosition(160, 30 + i * 80 + sliderScroll);
            slider.setSize(sf::Vector2f(130.f / 5 * ai->getBias(layer - 1, neur), 50));
            window.draw(slider);  

            std::string str("bias(" + std::to_string(layer + 1) + ", " + std::to_string(neur+1) + "): ");
            std::string value(std::to_string((round(ai->getBias(layer - 1, neur)*1000))/1000)); 
			value.erase(value.end()-1);
            value.erase(value.end()-2);
            text.setString(str + value);
            text.setOrigin(text.getGlobalBounds().width/2, text.getGlobalBounds().height/2);
            text.setPosition(160, 30 + i * 80 + sliderScroll + 22);
            window.draw(text);
            i++;
		}
	}
    
    text.setCharacterSize(60);
	text.setScale(0.25, 0.25);
}

void Game::initAi()
{
    aiSize.clear();
    aiSize.push_back(2);
    std::string cmd;
    do {
        std::cout << "enter the desired network size, layer by layer\n"; 
        std::cout << "(enter \'start\' to continue with the selected size)\n"; 
        std::cout << "current network's size: ";
        for (int i = 0; i < aiSize.size(); i++)
        {
            std::cout << aiSize[i] << ", ";
        }
        std::cout << "_. next layer's size: ";

        std::cin >> cmd;
        std::string::const_iterator it = cmd.begin();
        while (it != cmd.end() && std::isdigit(*it)) ++it;
        if (!cmd.empty() && it == cmd.end())
        {
            aiSize.push_back(stoi(cmd));
        }  

        std::cout << "\n";
        //system("CLS");
    } while (cmd != "start");

    
    do {
        std::cout << "current network's size: ";
        for (int i = 0; i < aiSize.size(); i++)
        {
            std::cout << aiSize[i] << ", ";
        }
        std::cout << "_.\n"; 

        std::cout << "do you want to create an RBG image (\'rgb\') or a black and white one (\'bw\')? ";
        std::cin >> cmd;

        if (cmd != "rgb" && cmd != "bw")
        {
            std::cout << "\n";
            //system("CLS");
        }
    } while (cmd != "rgb" && cmd != "bw");

    std::cout << "final network's size: ";
    for (int i = 0; i < aiSize.size(); i++)
    {
        std::cout << aiSize[i] << ", ";
    }

    if (cmd == "rgb")
    {
        aiSize.push_back(3);
        std::cout << "3.\n";        
    }
    else
    {
        aiSize.push_back(2);
        std::cout << "2.\n";        
    }

    ai = new Ai(aiSize);
}

void Game::setView()
{
	float width = 300 * (aiSize.size() - 1) + 240;
	float prop = 1600 / 1080.f;

	if (nnHeight * prop > width)
	{
		nn.setView(sf::View(sf::Vector2f(width/2, nnHeight/2.f), sf::Vector2f(nnHeight * prop, nnHeight)));
	}
	else
	{
		nn.setView(sf::View(sf::Vector2f(width/2, nnHeight/2.f), sf::Vector2f(width, width / prop)));
	}
}