// Content is accessed from the HTML relative to this path
#define CROW_STATIC_DIRECTORY "../app/"
#define CROW_ENABLE_SSL
#include <crow.h>
#include <iostream>

#include <fstream>
#include <sstream>
#include <string>

#include <inja/inja.hpp> // html template engine
#include <nlohmann/json.hpp>
#include <xxh3.h>
#include <xxhash.h> // hashing

void LoadJSON(const char *filename, nlohmann::json &data) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    throw std::exception("failed to open json file");
  }
  // parse json
  data = nlohmann::json::parse(ifs, nullptr, false, true);

  if (data.is_discarded()) {
    throw std::runtime_error("Failed to load/parse json");
  }
}

int main() {
  crow::SimpleApp app;

  // Load the config file
  nlohmann::json conf;
  LoadJSON("../../app/config.jsonc", conf);

  const auto default_and_show_error = [](std::string_view defStr) {
    std::cerr << "SSL files not specified in config.jsonc or corrupted "
                 "config.jsonc file"
              << std::endl;
    return std::string(defStr.data());
  };

  // check if ssl_cert and ssl_key are present, check before access
  if (!conf.contains("ssl_cert"))
    conf["ssl_cert"] = default_and_show_error("../../app/localhost.pem");
  if (!conf.contains("ssl_key"))
    conf["ssl_key"] = default_and_show_error("../../app/localhost.key");

  // Routes
  CROW_ROUTE(app, "/").name("home")(
      [](crow::response &res) { res.redirect("/H64"); });

  CROW_CATCHALL_ROUTE(app)([](crow::response &res) { res.redirect("/H64"); });

  // hashing routes, match ints
  CROW_ROUTE(app, "/H<int>")
  ([](const crow::request &req, crow::response &res, int hash_width) {
    inja::Environment env;
    inja::Template tpl =
        env.parse_template("../../app/templates/index.tpl.html");
    if (hash_width != 32 && hash_width != 64 && hash_width != 128) {
      res.redirect("/"); // redirect to home
    }
    // Build data
    inja::json page_data;
    page_data["page_title"] = "Hash any string";
    page_data["hash_width"] = hash_width;
    res.write(env.render(tpl, page_data));
    res.end();
  });

  CROW_ROUTE(app, "/H<int>")
      .methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)(
          [](const crow::request &req, crow::response &res, int hash_width) {
            // Load the index template
            inja::Environment env;
            inja::Template tpl =
                env.parse_template("../../app/templates/result.tpl.html");

            // Retrieve the data from the body of the POST request
            std::string body = req.body;

            // Find the position of "=" in the body
            size_t equalSignPos = body.find('=');
            std::string user_str;

            // req.get_body_params().get("string_to_hash"); // will be available
            // in 1.1

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

            // Build data
            inja::json page_data;
            page_data["page_title"] = "Your hash result";
            page_data["hash_width"] =
                hash_width; // we will use this to set the active item
            page_data["string_hash"] = ss.str(); // append result of hashing

            // Render Content
            res.write(env.render(tpl, page_data));
            res.end();
          }

      );

  // stop server
  CROW_ROUTE(app, "/stop").name("stop")([&app](crow::response &res) {
    app.stop();
  });

  // bind and run
  app.bindaddr(conf["server_ip"])
      .port(conf["server_port"])
      .ssl_file(conf["ssl_cert"], conf["ssl_key"])
      .multithreaded()
      .run_async();

  return 0;
}