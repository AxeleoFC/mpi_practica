#include <iostream>
#include <SFML/Graphics.hpp>
#include <fmt/core.h>
#include <chrono>
#include <vector>
#include <mpi.h>
static int rgba=4;
namespace ch=std::chrono;

std::tuple<sf::Uint8, sf::Uint8, sf::Uint8>
process_pixel_sobel(const sf::Uint8 *image, int width, int height, int pos) {
    int r_x = 0;
    int g_x = 0;
    int b_x = 0;

    int r_y = 0;
    int g_y = 0;
    int b_y = 0;

    int sobel_x[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int sobel_y[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int index = (pos * 4) + (i * 4) + (j * width * 4);

            if (index >= 0 && index <= width * height * 4) {
                int matrixIndex = (i + 1) * 3 + (j + 1);

                int weight_x = sobel_x[matrixIndex];
                int weight_y = sobel_y[matrixIndex];

                r_x += image[index] * weight_x;
                g_x += image[index + 1] * weight_x;
                b_x += image[index + 2] * weight_x;

                r_y += image[index] * weight_y;
                g_y += image[index + 1] * weight_y;
                b_y += image[index + 2] * weight_y;
            }
        }
    }

    int r = std::clamp((std::abs(r_x) + std::abs(r_y)) / 2, 0, 255);
    int g = std::clamp((std::abs(g_x) + std::abs(g_y)) / 2, 0, 255);
    int b = std::clamp((std::abs(b_x) + std::abs(b_y)) / 2, 0, 255);

    return {r, g, b};
}

std::vector<sf::Uint8 > bordesSobel(const sf::Uint8 * img, int width, int height){
    std::vector<sf::Uint8> retorno(height*width*rgba);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            auto [r, g, b] = process_pixel_sobel(img, width, height, j * width + i);
            int index = (j * width + i) * rgba;
            retorno[index] = r;
            retorno[index + 1] = g;
            retorno[index + 2] = b;
            retorno[index + 3] = 255;
        }
    }
    return retorno;
}

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

int main(int argc, char** argv){
    MPI_Init(&argc,&argv);
    int rank,nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    sf::Image image;
    int width, height;
    int filas_por_ranks;
    int filas_r;
    std::vector<sf::Uint8 > imgFinal;

    if(rank==0){
        image.loadFromFile("C:/Users/Akuseru/CLionProjects/mpiEx/img_3.png");
        width=image.getSize().x;
        height=image.getSize().y;
        filas_por_ranks=std::ceil((double )height/nprocs);
        imgFinal.resize(width*height*rgba);
    }
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&filas_por_ranks, 1,MPI_INT,0,MPI_COMM_WORLD);

    int inicio_filas=rank*filas_por_ranks;
    int fin_filas = inicio_filas+filas_por_ranks;

    if(rank==nprocs-1){
        fin_filas=height;
    }
    filas_r=fin_filas-inicio_filas;
    fmt::println("Filas por proceso: {}, inicio_filas: {}, fin_filas: {}", filas_r, inicio_filas, fin_filas);
    int pixel_por_filas = width*rgba;

    std::vector<sf::Uint8> buffer((fin_filas-inicio_filas)*pixel_por_filas);

    MPI_Scatter(image.getPixelsPtr(), pixel_por_filas * filas_r,MPI_UNSIGNED_CHAR,
                buffer.data(), pixel_por_filas * filas_r,MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    std::vector<sf::Uint8> sobelPar= bordesSobel(buffer.data(),width,filas_r);
    fmt::println("Datos totales s: {}", buffer.size());
    fmt::println("Datos totales r: {}", sobelPar.size());

    MPI_Gather(sobelPar.data(), pixel_por_filas*filas_r,MPI_UNSIGNED_CHAR,
               imgFinal.data(), pixel_por_filas*filas_r,MPI_UNSIGNED_CHAR,
               0,MPI_COMM_WORLD);

    MPI_Finalize();

    if(rank==0){
        fmt::println("Datos totales: {}", imgFinal.size());
        sf::Image bordesImg;
        bordesImg.create(width,height,imgFinal.data());
        //image.create(width,height,sobel.data());
        abrirImg(image,bordesImg);
    }
    return 0;

}