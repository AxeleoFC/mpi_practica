#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <fmt/core.h>
#include <chrono>
#include <mpi.h>

static int image_width;
static int image_height;
static int rgba=4;

std::vector<sf::Uint8> espejoImag(const sf::Uint8 * img, int width, int height){
    std::vector<sf::Uint8> buffer(width * height * rgba);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int index1 = (i * width + j) * rgba;
            int index2 = (i * width + (width - j - 1)) * rgba;
            buffer[index1] = img[index2];
            buffer[index1 + 1] = img[index2 + 1];
            buffer[index1 + 2] = img[index2 + 2];
            buffer[index1 + 3] = 255;
        }
    }
    return buffer;
}

void abrirImg(sf::Image imaV, const sf::Uint8 * vectImg) {
    sf::Image image=imaV;

    sf::Text text;
    sf::Font font;
    {
        font.loadFromFile("C:/Users/Akuseru/CLionProjects/mpiEx/arial.ttf");
        text.setFont(font);
        text.setString("");
        text.setCharacterSize(24); // in pixels, not points!
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);
        text.setPosition(10,10);
    }

    sf::Text textOptions;
    {
        font.loadFromFile("C:/Users/Akuseru/CLionProjects/mpiEx/arial.ttf");
        textOptions.setFont(font);
        textOptions.setCharacterSize(24);
        textOptions.setFillColor(sf::Color::White);
        textOptions.setStyle(sf::Text::Bold);
        textOptions.setString("OPTIONS: [R] Reset [B] Blur");
    }

    image_width = image.getSize().x;
    image_height = image.getSize().y;

    sf::Texture texture;
    texture.create(image_width, image_height);
    texture.update(image.getPixelsPtr());

    int w = 800;
    int h = 450;

    sf::RenderWindow  window(sf::VideoMode(w, h), "MPI Blur example");

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
    int fps = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if(event.type==sf::Event::KeyReleased) {
                if(event.key.scancode==sf::Keyboard::Scan::R) {
                    texture.update(image.getPixelsPtr());
                }
                else if(event.key.scancode==sf::Keyboard::Scan::B) {
                    texture.update(vectImg);
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
            fps = frames;
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


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // variables
    sf::Image image;
    int width, height;
    int filas_por_proceso;
    int filas_r;
    std::vector<sf::Uint8> imagenEspejo;

    if (rank == 0) {
        image.loadFromFile("C:/Users/Akuseru/CLionProjects/mpiEx/img.jpg");
        width = image.getSize().x;
        height = image.getSize().y;
        filas_por_proceso = std::ceil((double) height / nprocs);
        imagenEspejo.resize(width * height * rgba);
    }

    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&filas_por_proceso, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int inicio_filas = rank * filas_por_proceso;
    int fin_filas = inicio_filas + filas_por_proceso;

    if (rank == nprocs - 1) {
        fin_filas = height;
    }
    filas_r=fin_filas-inicio_filas;

    fmt::println("Filas por proceso: {}, inicio_filas: {}, fin_filas: {}", filas_r, inicio_filas, fin_filas);

    int pixeles_por_fila = width * rgba;

    std::vector<sf::Uint8> buffer((fin_filas - inicio_filas) * pixeles_por_fila);

    MPI_Scatter(image.getPixelsPtr(), pixeles_por_fila * filas_r, MPI_UNSIGNED_CHAR,
                buffer.data(), pixeles_por_fila * filas_r, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    std::vector<sf::Uint8> espejo= espejoImag(buffer.data(), width, filas_r);

    MPI_Gather(espejo.data(), pixeles_por_fila * filas_r, MPI_UNSIGNED_CHAR,
               imagenEspejo.data(), pixeles_por_fila * filas_r, MPI_UNSIGNED_CHAR,
               0, MPI_COMM_WORLD);

    MPI_Finalize();
    if(rank==0){
        abrirImg(image, imagenEspejo.data());
    }

    return 0;
}
