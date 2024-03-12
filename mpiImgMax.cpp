#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <fmt/core.h>
#include <mpi.h>

static int rgba=4;

void abrirImg(sf::Image imaV, sf::Image imaVect) {
    sf::Image image = imaV;
    sf::Image vectImage = imaVect;

    sf::Text text;
    sf::Font font;
    {
        font.loadFromFile("../arial.ttf");
        text.setFont(font);
        text.setString("");
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);
        text.setPosition(10,10);
    }

    sf::Text textOptions;
    {
        font.loadFromFile("../arial.ttf");
        textOptions.setFont(font);
        textOptions.setCharacterSize(24);
        textOptions.setFillColor(sf::Color::White);
        textOptions.setStyle(sf::Text::Bold);
        textOptions.setString("OPTIONS: [R] Reset [B] Blur");
    }

    int image_width = image.getSize().x;
    int image_height = image.getSize().y;

    sf::Texture texture;
    texture.create(image_width, image_height);
    texture.update(image.getPixelsPtr());

    int w = 800;
    int h = 450;

    sf::RenderWindow  window(sf::VideoMode(w, h), "MPI example");

    sf::Sprite sprite;
    {
        sprite.setTexture(texture);

        float scaleFactorX = w * 1.0 / image.getSize().x;
        float scaleFactorY = h * 1.0 / image.getSize().y;
        sprite.scale(scaleFactorX, scaleFactorY);
    }

    sf::Clock clock;

    sf::Clock clockFrames;
    int frames = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if(event.type==sf::Event::KeyReleased) {
                if(event.key.scancode==sf::Keyboard::Scan::R) {
                    texture.create(image_width, image_height);
                    texture.update(image.getPixelsPtr());
                }
                else if(event.key.scancode==sf::Keyboard::Scan::B) {
                    texture.create(vectImage.getSize().x, vectImage.getSize().y);
                    texture.update(vectImage.getPixelsPtr());
                }
            }
            else if(event.type==sf::Event::Resized) {
                float scaleFactorX = event.size.width *1.0 / image.getSize().x;
                float scaleFactorY = event.size.height *1.0 /image.getSize().y;

                sprite = sf::Sprite();
                sprite.setTexture(texture);
                sprite.scale(scaleFactorX, scaleFactorY);
            }
        }

        if(clockFrames.getElapsedTime().asSeconds()>=1.0) {
            frames = 0;
            clockFrames.restart();
        }
        frames++;

        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.draw(text);
        window.draw(textOptions);
        window.display();
    }
}


std::tuple<sf::Uint8, sf::Uint8, sf::Uint8> calcular_max(const sf::Uint8 *image, int width, int height, int x, int y) {
    std::vector<sf::Uint8> rojo;
    std::vector<sf::Uint8> verde;
    std::vector<sf::Uint8> azul;

    for(int i=x-1;i<=x+1;i++) {
        for(int j=y-1;j<=y+1;j++) {
            int index = (j * width + i)*rgba;
            if(i>=0 && i<width && j>=0 && j<height) {
                rojo.push_back(image[index]);
                verde.push_back(image[index + 1]);
                azul.push_back(image[index + 2]);
            }
        }
    }
    auto max_rojo = *std::max_element(rojo.begin(), rojo.end());
    auto max_verde = *std::max_element(verde.begin(), verde.end());
    auto max_azul = *std::max_element(azul.begin(), azul.end());

    return std::make_tuple(max_rojo, max_verde, max_azul);
}

std::vector<sf::Uint8> maxImg(const sf::Uint8* img, int width, int height) {
    std::vector<sf::Uint8> buffer(width * height * rgba);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            auto [r, g, b] = calcular_max(img, width, height, i,j);
            int index = (j * width + i) * rgba;
            buffer[index] = r;
            buffer[index + 1] = g;
            buffer[index + 2] = b;
            buffer[index + 3] = 255;
        }
    }
    return buffer;
}

int main(int argc, char** argv){
    MPI_Init(&argc,&argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    sf::Image image;
    int width, height;
    int filas_por_ranks;
    int filas_r;
    std::vector<sf::Uint8> max;

    if (rank==0){
        image.loadFromFile("C:/Users/Akuseru/CLionProjects/mpiEx/img_3.png");
        width=image.getSize().x;
        height=image.getSize().y;
        filas_por_ranks=std::ceil((double )height/nprocs);
        max.resize(width*height*rgba);
    }
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&filas_por_ranks, 1,MPI_INT,0,MPI_COMM_WORLD);

    int inicio_filas = rank*filas_por_ranks;
    int fin_filas = inicio_filas+filas_por_ranks;

    if(rank==nprocs-1){
        fin_filas=height;
    }
    filas_r=fin_filas-inicio_filas;
    fmt::println("Filas por proceso={}, inicio filas={}, fin filas={}", filas_r,inicio_filas,fin_filas);
    int pixel_por_filas=width*rgba;

    std::vector<sf::Uint8> buffer((fin_filas-inicio_filas)*pixel_por_filas);

    MPI_Scatter(image.getPixelsPtr(), pixel_por_filas*filas_r, MPI_UNSIGNED_CHAR,
                buffer.data(), pixel_por_filas*filas_r,MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    std::vector<sf::Uint8> maxParci= maxImg(buffer.data(),width,filas_r);

    MPI_Gather(maxParci.data(), pixel_por_filas*filas_r, MPI_UNSIGNED_CHAR,
               max.data(), pixel_por_filas*filas_r, MPI_UNSIGNED_CHAR,
               0, MPI_COMM_WORLD);
    MPI_Finalize();
    if(rank==0){
        sf::Image maxImge;
        maxImge.create(width,height,max.data());
        abrirImg(image,maxImge);
    }

    return 0;
}