// Content is accessed from the HTML relative to this path
#define CROW_STATIC_DIRECTORY "../../app/"
#define CROW_ENABLE_SSL
#define CROW_JSON_NO_ERROR_CHECK // error handling done by us
#include <crow.h>
#include <iostream>

#include <fstream>
#include <sstream>
#include <string>
#include <xxh3.h>
#include <xxhash.h> // hashing

crow::json::rvalue LoadJSON(std::string filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    return crow::json::rvalue{};
    std::cerr << "failed to open json file" << std::endl;
  }

  std::string jsonStr((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  return crow::json::load(jsonStr);
}

int main() {
  // crow app
  crow::SimpleApp app;

  // Load the config file
  crow::json::rvalue config = LoadJSON("../../app/config.jsonc");

  // Setup templates
  crow::mustache::set_global_base("../../app/templates");

  // Routes
  CROW_ROUTE(app, "/").name("home")(
      [](crow::response &res) { res.redirect("/H64"); });

  CROW_CATCHALL_ROUTE(app)([](crow::response &res) { res.redirect("/H64"); });

  // hashing routes
  CROW_ROUTE(app, "/H<int>")
  ([](const crow::request &req, crow::response &res, int hash_width) {
    crow::mustache::template_t page =
        crow::mustache::load("./index_mustache.html");

    if (hash_width != 32 && hash_width != 64 && hash_width != 128) {
      res.redirect("/"); // redirect to home
    }

    // Build data
    crow::mustache::context ctx;
    // Select active item
    ctx[std::string("isActive") + std::to_string(hash_width)] = "active";

    ctx["page_title"] = "Hash any string";
    ctx["hash_width"] = hash_width;

    // Render Content
    res.write(page.render_string(ctx));
    res.end();
  });

  CROW_ROUTE(app, "/H<int>")
      .methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)(
          [](const crow::request &req, crow::response &res, int hash_width) {
            crow::mustache::template_t page =
                crow::mustache::load("./result_mustache.html");

            std::cout << "The page content is below:\n" << page.render_string();

            // Retrieve the data from the body of the POST request
            std::string body = req.body;

            // Find the position of "=" in the body
            size_t equalSignPos = body.find('=');
            std::string user_str;

            // req.get_body_params().get("string_to_hash"); // will be in v1.1

            // Check if "=" is found and if there's content after it
            if (equalSignPos != std::string::npos &&
                equalSignPos < body.length() - 1) {
              // Extract the value after "="
              user_str = body.substr(equalSignPos + 1);
            } else {
              res.code = 400; // Bad Request
              res.end();
              res.redirect("/");
            }

            // Setup stringstream for hashing
            std::stringstream ss;
            ss << std::hex << std::setfill('0');

            // hashing
            switch (hash_width) {
            case 32:
              ss << std::setw(8)
                 << XXH32(user_str.c_str(), user_str.length(), 0);
              break;
            case 64:
              ss << std::setw(16)
                 << XXH64(user_str.c_str(), user_str.length(), 0);
              break;
            case 128: {
              auto hash = XXH128(user_str.c_str(), user_str.length(), 0);
              ss << std::setw(16) << hash.low64 << hash.high64;
              break;
            }
            default:
              res.redirect("/"); // redirect to home, stop on any other hash
            }

            crow::mustache::context ctx;
            ctx[std::string("isActive") + std::to_string(hash_width)] =
                "active";
            ctx["page_title"] = "Your hash result";
            ctx["hash_width"] = hash_width;
            ctx["string_hash"] = ss.str();

            res.write(page.render_string(ctx));
            res.end();
          }

      );

  // stop server
  CROW_ROUTE(app, "/stop").name("stop")([&app](crow::response &res) {
    app.stop();
  });

  const auto default_and_show_error = [](std::string_view defStr) {
    std::cerr << "Config info not specified in config.jsonc or corrupted "
                 "config.jsonc file"
              << std::endl;
    return std::string(defStr.data());
  };

  // bind and run
  // check if ssl_cert and ssl_key are present, check before access
  app.bindaddr(config.has("server_ip") ? config["server_ip"].s()
                                       : default_and_show_error("0.0.0.0"))
      .port(config.has("server_port") ? config["server_port"].i() : 443)
      .ssl_file(config.has("ssl_cert")
                    ? config["ssl_cert"].s()
                    : default_and_show_error("../../app/localhost.pem"),
                config.has("ssl_key")
                    ? config["ssl_key"].s()
                    : default_and_show_error("../../app/localhost.key"))
      .multithreaded()
      .run_async();

  return 0;
}
