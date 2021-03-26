$(document).ready(() => {
  let input = $(".form #emailinput");
  let send = $(".form #send");
  let spinner = $(".loading");
  let confirmation = $(".confirmation");
  let regex = /[^@]+@[^@]+\.[^@]+/g;
  let url = "https://vaccinocovid19.live/post/newsletter";

  const removeDefaultText = () => {
    text = input.val();
    if (text.includes(default_text)) {
      // remove default text
      input.val(text.replace(default_text, ""));
      input.css({ "opacity": 1 });
    } else if (text === "") {
      // reset to default
      input.val(default_text);
      input.css({ "opacity": 0.5 });
    }
  };

  const showError = (error) => {
    confirmation.css({ "background-color": "Brown" });
    confirmation.html("<p>Errore interno del server.<br>Si prega di provare più tardi.</p>");
    showConfirmation();
  };

  const showSuccess = (success) => {
    if (success.response != 200) {
      showError();
      return;
    }

    confirmation.css({ "background-color": "lightgreen" });
    confirmation.html("<p>La tua email è stata salvata.<br>Verrai presto ricontattato!</p>");
    showConfirmation();
  };

  const showConfirmation = () => {
    confirmation.css({ "display": "flex" });
    confirmation.animate({
      "height": "4rem"
    }, 500);
  };

  // preload text
  let default_text = "Inserisci la tua email";
  input.val(default_text);
  input.css({ "opacity": 0.5 });

  // remove preloaded text
  input.on("input", removeDefaultText);
  // i cannot make this work with "on" so i bind it to a different event
  input.click(removeDefaultText);

  input.on("input", () => {
    send.attr("disabled", !input.val().match(regex));
  });

  send.click(() => {
    confirmation.css({ "display": "none" });
    spinner.css({ "display": "block" });

    let data = {
      "email": input.val()
    };

    $.ajax({
      url: url,
      dataType: "json",
      type: "post",
      contentType: 'application/json',
      data: JSON.stringify(data),
      processData: false,
      cache: false,
      success: function (data) {
        spinner.css({ "display": "none" });
        showSuccess(data);
      },
      error: function (error) {
        spinner.css({ "display": "none" });
        showError(error);
      }
    });

  });
});