#include <chrono>
#include <fmt/core.h>
#include <string>
#include <string_view>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "conf.h"
#include "image.h"
#include "util.h"

auto constexpr inline filename_def = "mandelbrot.pgm";

auto static glfw_error_callback(int const error, char const* const desc) noexcept -> void {
  fmt::print(stderr, "GLFW Error: {}: {}\n", error, desc);
}

auto main(i32 const argc, char const* const* const argv) -> int {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    fmt::print("Failed to initialize GLFW\n");
    return -1;
  }

  auto const glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto window = glfwCreateWindow(1280, 720, "Example", nullptr, nullptr);
  if (!window) {
    fmt::print("Failed to create GLFW window\n");
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // vsync

  if (glewInit() != GLEW_OK) {
    fmt::print("Failed to initialize GLEW\n");
    return -1;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto io = ImGui::GetIO();

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  auto show_demo_window = true;
  auto const clear_color = ImVec4{0.45f, 0.55f, 0.60f, 1.00f};

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Render();

    int disp_w, disp_h;
    glfwGetFramebufferSize(window, &disp_w, &disp_h);
    glViewport(0, 0, disp_w, disp_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  // auto img = Image{{}};
}
