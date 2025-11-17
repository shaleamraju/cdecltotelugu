$(document).ready(function () {
  var inputTimeout;
  $("#query").on("input", function () {
    clearTimeout(inputTimeout);
    var query = $(this).val();
    if (query.length === 0 || query.length >= 3) {
      inputTimeout = setTimeout(function () {
        $("#spinner").show();
        $.ajax({
          data: {
            query: query || "h",
          },
          type: "POST",
          url: "/",
        }).done(function (data) {
          $("#output").text(data.output).show();
          setTimeout(function () {
            $("#spinner").hide();
          }, 100);
        });
      }, 500);
    }
  });

  $("#sourceCodeButton").on("click", function () {
    window.location.href = "https://github.com/Raphael-08/Cdecl-telugu";
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
