// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"
#include <stdio.h>
#include <string>
#include <regex>
#include <string.h>

#include <thread>

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif
#include <random>
#include <iostream>
#include <winsock.h>
#include <regex>



static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x, y));
        else {
            Data[Offset] = ImVec2(x, y);
            Offset = (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset = 0;
        }
    }
};

struct RollingBuffer {
    float Span;
    ImVector<ImVec2> Data;
    RollingBuffer() {
        Span = 10.0f;
        Data.reserve(2000);
    }
    void AddPoint(float x, float y) {
        float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};

// Main code
int main(int, char**)
{



    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // IP adresi saklanacak de�i�ken
    char ipAddress[16] = "";
    bool PAGE = false;
    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = NULL;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
      

        //// IP adresi giri�i ve "Login" d��mesi
        //ImGui::SetNextWindowSize(ImVec2(720, 320));
        //ImGui::Begin("PAGE", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        //ImGui::InputText("IP Adresi", ipAddress, 32);
        //if (ImGui::Button("Login")) {

        //    // Login d��mesine t�kland���nda, saklanan IP adresi ile di�er sayfaya ge�me
        //    // �rnek olarak, saklanan IP adresini konsola yazd�rma:
        //    std::cout << "Girilen IP Adresi: " << ipAddress << std::endl;

        //    // Yeni sayfay� a��n ve IP adresi bilgisini aktarma
        //    if (strlen(ipAddress) > 0) {
        //        ImGui::OpenPopup("Yeni Sayfa");
        //    }

        //}




        //------------------------------------------------------------------------------------------
        //   // IP adresi giri�i ve "Login" d��mesi
        //ImGui::SetNextWindowSize(ImVec2(720, 320));
        //ImGui::Begin("PAGE", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        //// IP adresi giri� kutusu
        //ImGui::InputText("IP Adresi", ipAddress, 32);

        //if (ImGui::Button("Login")) {
        //    // IP adresi do�rulama
        //    bool validIp = true;
        //    std::string ipAddressString = std::string(ipAddress);
        //    std::istringstream iss(ipAddressString);
        //    std::string token;
        //    while (std::getline(iss, token, '.')) {
        //        try {
        //            int octet = std::stoi(token);
        //            if (octet < 0 || octet > 255) {
        //                validIp = false;
        //                break;
        //            }
        //        }
        //        catch (const std::exception&) {
        //            validIp = false;
        //            break;
        //        }
        //    }

        //    if (!validIp) {
        //        ImGui::OpenPopup("Hata");
        //    }
        //    else {
        //        // IP adresini sakla
        //        std::cout << "Girilen IP Adresi: " << ipAddress << std::endl;
        //        if (strlen(ipAddress) > 0) {
        //            ImGui::OpenPopup("Yeni Sayfa");
        //        }
        //    }
        //}

        //// Hata mesaj�
        //if (ImGui::BeginPopupModal("Hata", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        //    ImGui::Text("L�tfen ge�erli bir IP adresi girin.");
        //    if (ImGui::Button("Tamam")) {
        //        ImGui::CloseCurrentPopup();
        //    }
        //    ImGui::EndPopup();
        //}
        //
        //
        //
        // ----------------------------------------------------------------------------------------------------------

        ImGui::SetNextWindowSize(ImVec2(420, 320));
        ImGui::Begin("PAGE", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        style.WindowRounding = 0.9f; // Pencere k��elerinin yuvarlanma miktar�
        // style.Colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.0f); // Pencere arka plan rengi

        colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.95f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
        colors[ImGuiCol_Button] = ImVec4(0.67f, 0.40f, 0.40f, 0.60f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.67f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        ImGui::InputText("IP Adresi", ipAddress, 32); // IP adresi giri� kutusu

        // IP adresi giri�i ve "Login" d��mesi
        if (ImGui::Button("Login")) {
            // IP adresi do�rulama
            std::regex ip_regex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
            if (!std::regex_match(ipAddress, ip_regex)) {
                ImGui::OpenPopup("Uyari");
            }
            else {
                // Login d��mesine t�kland���nda, saklanan IP adresi ile di�er sayfaya ge�me
                std::cout << "Girilen IP Adresi:" << ipAddress << std::endl;
                ImGui::OpenPopup("Yeni Sayfa");
            }
        }

        // IP adresi uyar� mesaj�
        if (ImGui::BeginPopupModal("Uyari", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Lutfen gecerli bir IP adresi girin.");
            if (ImGui::Button("Tamam")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        // Yeni sayfa penceresi
        if (ImGui::BeginPopupModal("Yeni Sayfa")) {
            ImGui::SetNextWindowSize(ImVec2(720, 720));
            // Yeni sayfa i�eri�i ve IP adresi bilgisi
           
            static ScrollingBuffer sdata1, sdata2;
            static RollingBuffer rdata1, rdata2;
            static float t = 0;
            t += ImGui::GetIO().DeltaTime;
            sdata1.AddPoint(t, 2355 * 0.0005f);
            sdata2.AddPoint(t, 5555 * 0.0005f);

            rdata1.AddPoint(t, 2355 * 0.0005f);
            rdata2.AddPoint(t, 5555 * 0.0005f);


            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(2355 * 0.0005, 5555 * 0.0005);

            //for (int i = 0; i < 10; ++i) {
            //    t += ImGui::GetIO().DeltaTime;
            //    sdata1.AddPoint(t, dis(gen));
            //    sdata2.AddPoint(t, dis(gen));
            //}

            //for (int i = 0; i < 20; ++i) {
            //    t += ImGui::GetIO().DeltaTime;
            //    rdata1.AddPoint(t, dis(gen));
            //    rdata2.AddPoint(t, dis(gen));
            //}

            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;


            static float history = 10.0f;

            ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
            rdata1.Span = history;
            rdata2.Span = history;


            if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, 150))) {
                ImPlot::SetupAxes(NULL, NULL, flags, flags);
                ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
                ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
                ImPlot::PlotShaded("Mouse X", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), -INFINITY, 0, sdata1.Offset, 2 * sizeof(float));
                ImPlot::PlotLine("Mouse Y", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), 0, sdata2.Offset, 2 * sizeof(float));
                ImPlot::EndPlot();
            }


            if (ImPlot::BeginPlot("##Rolling", ImVec2(-1, 150))) {
                ImPlot::SetupAxes(NULL, NULL, flags, flags);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
                ImPlot::PlotLine("Mouse X", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
                ImPlot::PlotLine("Mouse Y", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
                ImPlot::EndPlot();
            }
            ImGui::Text("Girilen IP Adresi: %s", ipAddress); // Girilen IP adresini g�stermek i�in metin kutusu olu�turma

            // Yeni sayfa butonlar�
            if (ImGui::Button("Tamam")) {
                ImGui::CloseCurrentPopup(); // Pencereyi kapat�n
            }
            ImGui::EndPopup();  
        }  ImGui::End();
        //-----------------------------------draw graphics------------------------------------------------------- 

        
        



        //-------------------------------------------------------------------------------------------------------


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
