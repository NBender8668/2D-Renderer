#include "Canvas.h"
#include "Image.h"



SDL_Color PixelToColor(const pixel_t& pixel)
{

    SDL_Color color;

    color.r = pixel >> 24;
    color.g = (pixel >> 16) & 0xff;
    color.b = (pixel >> 8) & 0xff;
    color.a = pixel & 0xff;

    return color;


}

pixel_t ColorToPixel(const SDL_Color& color)
{
    return (color.r << 24 | color.g << 16 | color.b << 8 | color.a);
};

Canvas::Canvas(SDL_Renderer* renderer, int width, int height)
{
    m_width = width;
    m_height = height;

    m_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        width, height);

    m_buffer.resize(width * height);
}

Canvas::~Canvas()
{
    SDL_DestroyTexture(m_texture);
}

void Canvas::Update()
{
    SDL_UpdateTexture(m_texture, nullptr, m_buffer.data(), m_width * sizeof(pixel_t));
}

void Canvas::Draw(SDL_Renderer* renderer)
{
    SDL_RenderCopy(renderer, m_texture, nullptr, nullptr);
}

void Canvas::Clear(const SDL_Color& color)
{
    std::fill(m_buffer.begin(), m_buffer.end(), (color.r << 24 | color.g << 16 | color.b << 8 | color.a));
}

void Canvas::DrawPoint(const SDL_Point& point, const SDL_Color& color)
{
    if (point.x < 0 || point.x >= m_width || point.y < 0 || point.y >= m_height) return;

    m_buffer[point.x + (point.y * m_width)] = (color.r << 24 | color.g << 16 | color.b << 8 | color.a);
}

void Canvas::DrawLine(const SDL_Point& v1, const SDL_Point& v2, const SDL_Color& color)
{
    SDL_Point p1 = v1;
    SDL_Point p2 = v2;

    int dx = p2.x - p1.x;//run
    int dy = p2.y - p1.y;//rise

    if (dx == 0)
    {
        if (p1.y > p2.y)std::swap(p1.y, p2.y);

        for (int y = p1.y; y < p2.y; y++)
        {
            DrawPoint({ p1.x, y }, color);
        }
    }
    else
    {
        float m = dy / static_cast<float>(dx);
        float b = p1.y - (m * p1.x);

        if (m <= 1.0f && m >= -1.0f) 
        {
            if (p1.x > p2.x) std::swap(p1.x, p2.x);
            for (int x = p1.x; x < p2.x; x++)
            {
                int y = static_cast<int>(round((m * x) + b));
                DrawPoint({ x, y }, color);
            }
        }
        else
        {
            if (p1.y > p2.y) std::swap(p1.y, p2.y);
            for (int y = p1.y; y < p2.y; y++)
            {
                int x = static_cast<int>(round((y - b) / m));
                DrawPoint({ x, y }, color);
            }
        }
    }

}

void Canvas::DrawLineList(const std::vector<SDL_Point> point, const SDL_Color& color)
{
    for (size_t i = 0; i < point.size() - 1; i++)
    {
        DrawLine(point[i], point[i + 1], color);
    }
}





void Canvas::DrawCircle(const SDL_Point& center, int radius, const SDL_Color& color)
{
    int x = 0, y = radius;
    int d = 3 - 2 * radius;
    DrawCirclePoint(center, { x, y }, color);
    while (y >= x)
    {
        // for each pixel we will 
        // draw all eight pixels 

        x++;

        // check for decision parameter 
        // and correspondingly  
        // update d, x, y 
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
        DrawCirclePoint(center, {x, y}, color);
        SDL_Delay(50);
    }
}

void Canvas::DrawRect(const SDL_Rect& rect, const SDL_Color& color)
{
   
    for (int y = rect.y ; y < rect.h ; y++) 
    { 
        for (int x = rect.x ; x < rect.w ; x++) 
        { 
            DrawPoint({ x, y }, color); 8;
        } 
    }
}

void Canvas::DrawTriangle(const SDL_Point& v1, const SDL_Point& v2, const SDL_Point& v3, const SDL_Color& color)
{
    DrawLine(v1, v2, color);
    DrawLine(v2, v3, color);
    DrawLine(v3, v1, color);
}

void Canvas::DrawImage(const Image* image, const SDL_Point& point)
{
    for (int y = 0; y < image->m_height; y++)
    {
        int screeny = point.y + y;

        for (int x = 0; x < image->m_width; x++)
        {
            int screenx = point.x + x;

            if (screenx < 0 || screenx >= m_width || screeny < 0 || screeny >= m_height) continue;

            SDL_Color dstColor = PixelToColor(m_buffer[screenx + (screeny * m_width)]);
            SDL_Color srcColor = PixelToColor(image->m_buffer[x + (y * (image->m_width))]);

            float alpha = srcColor.a / 255.0f;

            srcColor.r = (Uint8)(srcColor.r * alpha);
            srcColor.g = (Uint8)(srcColor.g * alpha);
            srcColor.b = (Uint8)(srcColor.b * alpha);

            dstColor.r = (Uint8)(dstColor.r * (1 - alpha));
            dstColor.g = (Uint8)(dstColor.g * (1 - alpha));
            dstColor.b = (Uint8)(dstColor.b * (1 - alpha));

            SDL_Color color;
            color.r = srcColor.r + dstColor.r;
            color.g = srcColor.g + dstColor.g;
            color.b = srcColor.b + dstColor.b;

            m_buffer[screenx + (screeny * m_width)] = ColorToPixel(color);
        }

    }

}

void Canvas::DrawImage(const Image* image, const SDL_Point& point, const SDL_Color& Key)
{
    pixel_t pixelKey = ColorToPixel(Key);

    for (int y = 0; y < image->m_height; y++)
    {
        int screeny = point.y + y;

        for (int x = 0; x < image->m_width; x++)
        {
            int screenx = point.x + x;

            if (screenx < 0 || screenx >= m_width || screeny < 0 || screeny >= m_height) continue;

            pixel_t pixel = image->m_buffer[x + (y * image->m_width)];
            if (pixel == pixelKey)continue;

            m_buffer[screenx + (screeny * m_width)] = pixel ;
        }

    }

}



void Canvas::ProcessInvert()
{
    for (pixel_t& pixel : m_buffer)
    {
        SDL_Color color = PixelToColor(pixel);

        color.r = 255 - color.r;
        color.g = 255 - color.g;
        color.b = 255 - color.b;

        pixel = ColorToPixel(color);

    }
}

void Canvas::ProcessMonochrome()
{
    for (pixel_t& pixel : m_buffer)
    {
        SDL_Color color = PixelToColor(pixel);

        int sum = color.r + color.g + color.b;

        int average = (color.r + color.g + color.b) / 3;

            color.r = average;
            color.g = average;
            color.b = average;
            pixel = ColorToPixel(color);
    }
}

void Canvas::ProcessNightVision()
{
    for (pixel_t& pixel : m_buffer)
    {
        SDL_Color color = PixelToColor(pixel);

        int sum = color.r + color.g + color.b;

        int average = (color.r + color.g + color.b) / 3;

        color.r = 0;
        color.g = average;
        color.b = 0;
        pixel = ColorToPixel(color);
    }
}

void Canvas::ProcessThreshold(const SDL_Color& threshold)
{
    for (pixel_t& pixel : m_buffer)
    {
        SDL_Color color = PixelToColor(pixel);

        color.r = (color.r >= threshold.r) ? color.r : 0;
        color.g = (color.g >= threshold.g) ? color.g : 0;
        color.b = (color.b >= threshold.b) ? color.g : 0;
        color.a = (color.a >= threshold.b) ? color.a : 0;

        pixel = ColorToPixel(color);
    }
}

void Canvas::ProcessBrightness(int brightness)
{
    for (pixel_t& pixel : m_buffer) 
    {
        SDL_Color color = PixelToColor(pixel);

        color.r = std::min(std::max((int)color.r + brightness, 0), 255);
        color.g = std::min(std::max((int)color.g + brightness, 0), 255);
        color.b = std::min(std::max((int)color.b + brightness, 0), 255);

        pixel = ColorToPixel(color);
    }
}

void Canvas::ProcessColor( int r, int g, int b)
{
    for (pixel_t& pixel : m_buffer)
    {
        SDL_Color color = PixelToColor(pixel);

        color.r = std::min(std::max((int)color.r + r, 0), 255);
        color.g = std::min(std::max((int)color.g + g, 0), 255);
        color.b = std::min(std::max((int)color.b + b, 0), 255);

        pixel = ColorToPixel(color);
    }

}


void Canvas::DrawCirclePoint(const SDL_Point& center, const SDL_Point& point, const SDL_Color& color)
{
    DrawPoint({ center.x + point.x, center.y + point.y }, color);
    DrawPoint({ center.x - point.x, center.y + point.y }, color);
    DrawPoint({ center.x + point.x, center.y - point.y }, color);
    DrawPoint({ center.x - point.x, center.y - point.y }, color);
    DrawPoint({ center.x + point.y, center.y + point.x }, color);
    DrawPoint({ center.x - point.y, center.y + point.x }, color);
    DrawPoint({ center.x + point.y, center.y - point.x }, color);
    DrawPoint({ center.x - point.y, center.y - point.x }, color);
}


