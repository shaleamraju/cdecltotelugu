$(document).ready(function () {
  var inputTimeout;
  $("#query").on("input", function () {
    clearTimeout(inputTimeout);
    var query = $(this).val();
    if (query.length === 0) {
      $("#output").text("").hide();
      $("#spinner").hide();
      return;
    }
    if (query.length >= 3) {
      inputTimeout = setTimeout(function () {
        $("#spinner").show();
        $.ajax({
          data: {
            query: query,
          },
          type: "POST",
          url: "/",
        }).done(function (data) {
          var message = data.output || data.error || "No output";
          $("#output").text(message).show();
        }).fail(function (xhr) {
          var message = "Server error";
          if (xhr.responseJSON && (xhr.responseJSON.output || xhr.responseJSON.error)) {
            message = xhr.responseJSON.output || xhr.responseJSON.error;
          }
          $("#output").text(message).show();
        }).always(function () {
          setTimeout(function () {
            $("#spinner").hide();
          }, 100);
        });
      }, 500);
    }
  });

  $("#sourceCodeButton").on("click", function () {
    window.location.href = "https://github.com/shaleamraju/cdecltotelugu";
  });
});

document.addEventListener("DOMContentLoaded", function () {
  const modeToggle = document.getElementById("dark-mode");
  const body = document.body;

  const isDarkMode = window.matchMedia("(prefers-color-scheme: dark)").matches;

  if (isDarkMode) {
    body.classList.add("dark-mode");
    modeToggle.checked = true;
  }

  modeToggle.addEventListener("change", () => {
    if (modeToggle.checked) {
      body.classList.add("dark-mode");
    } else {
      body.classList.remove("dark-mode");
    }
  });
});
