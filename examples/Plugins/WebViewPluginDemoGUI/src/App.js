/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

import "@fontsource/roboto/300.css";
import "@fontsource/roboto/400.css";
import "@fontsource/roboto/500.css";
import "@fontsource/roboto/700.css";

import Box from "@mui/material/Container";
import Checkbox from "@mui/material/Checkbox";
import Typography from "@mui/material/Typography";
import Container from "@mui/material/Container";
import Slider from "@mui/material/Slider";
import Button from "@mui/material/Button";
import CardActions from "@mui/material/CardActions";
import Snackbar from "@mui/material/Snackbar";
import IconButton from "@mui/material/IconButton";
import CloseIcon from "@mui/icons-material/Close";
import InputLabel from "@mui/material/InputLabel";
import MenuItem from "@mui/material/MenuItem";
import FormControl from "@mui/material/FormControl";
import Select from "@mui/material/Select";
import FormGroup from "@mui/material/FormGroup";
import FormControlLabel from "@mui/material/FormControlLabel";

import { React, useState, useEffect, useRef } from "react";
import PropTypes from "prop-types";

import * as Juce from "juce-framework-frontend";

import "./App.css";

// Custom attributes in React must be in all lower case
const controlParameterIndexAnnotation = "controlparameterindex";

function JuceSlider({ identifier, title }) {
  JuceSlider.propTypes = {
    identifier: PropTypes.string,
    title: PropTypes.string,
  };

  const sliderState = Juce.getSliderState(identifier);

  const [value, setValue] = useState(sliderState.getNormalisedValue());
  const [properties, setProperties] = useState(sliderState.properties);

  const handleChange = (event, newValue) => {
    sliderState.setNormalisedValue(newValue);
    setValue(newValue);
  };

  const mouseDown = () => {
    sliderState.sliderDragStarted();
  };

  const changeCommitted = (event, newValue) => {
    sliderState.setNormalisedValue(newValue);
    sliderState.sliderDragEnded();
  };

  useEffect(() => {
    const valueListenerId = sliderState.valueChangedEvent.addListener(() => {
      setValue(sliderState.getNormalisedValue());
    });
    const propertiesListenerId = sliderState.propertiesChangedEvent.addListener(
      () => setProperties(sliderState.properties)
    );

    return function cleanup() {
      sliderState.valueChangedEvent.removeListener(valueListenerId);
      sliderState.propertiesChangedEvent.removeListener(propertiesListenerId);
    };
  });

  function calculateValue() {
    return sliderState.getScaledValue();
  }

  return (
    <Box
      {...{
        [controlParameterIndexAnnotation]:
          sliderState.properties.parameterIndex,
      }}
    >
      <Typography sx={{ mt: 1.5 }}>
        {properties.name}: {sliderState.getScaledValue()} {properties.label}
      </Typography>
      <Slider
        aria-label={title}
        value={value}
        scale={calculateValue}
        onChange={handleChange}
        min={0}
        max={1}
        step={1 / (properties.numSteps - 1)}
        onChangeCommitted={changeCommitted}
        onMouseDown={mouseDown}
      />
    </Box>
  );
}

function JuceCheckbox({ identifier }) {
  JuceCheckbox.propTypes = {
    identifier: PropTypes.string,
  };

  const checkboxState = Juce.getToggleState(identifier);

  const [value, setValue] = useState(checkboxState.getValue());
  const [properties, setProperties] = useState(checkboxState.properties);

  const handleChange = (event) => {
    checkboxState.setValue(event.target.checked);
    setValue(event.target.checked);
  };

  useEffect(() => {
    const valueListenerId = checkboxState.valueChangedEvent.addListener(() => {
      setValue(checkboxState.getValue());
    });
    const propertiesListenerId =
      checkboxState.propertiesChangedEvent.addListener(() =>
        setProperties(checkboxState.properties)
      );

    return function cleanup() {
      checkboxState.valueChangedEvent.removeListener(valueListenerId);
      checkboxState.propertiesChangedEvent.removeListener(propertiesListenerId);
    };
  });

  const cb = <Checkbox checked={value} onChange={handleChange} />;

  return (
    <Box
      {...{
        [controlParameterIndexAnnotation]:
          checkboxState.properties.parameterIndex,
      }}
    >
      <FormGroup>
        <FormControlLabel control={cb} label={properties.name} />
      </FormGroup>
    </Box>
  );
}

function JuceComboBox({ identifier }) {
  JuceComboBox.propTypes = {
    identifier: PropTypes.string,
  };

  const comboBoxState = Juce.getComboBoxState(identifier);

  const [value, setValue] = useState(comboBoxState.getChoiceIndex());
  const [properties, setProperties] = useState(comboBoxState.properties);

  const handleChange = (event) => {
    comboBoxState.setChoiceIndex(event.target.value);
    setValue(event.target.value);
  };

  useEffect(() => {
    const valueListenerId = comboBoxState.valueChangedEvent.addListener(() => {
      setValue(comboBoxState.getChoiceIndex());
    });
    const propertiesListenerId =
      comboBoxState.propertiesChangedEvent.addListener(() => {
        setProperties(comboBoxState.properties);
      });

    return function cleanup() {
      comboBoxState.valueChangedEvent.removeListener(valueListenerId);
      comboBoxState.propertiesChangedEvent.removeListener(propertiesListenerId);
    };
  });

  return (
    <Box
      {...{
        [controlParameterIndexAnnotation]:
          comboBoxState.properties.parameterIndex,
      }}
    >
      <FormControl fullWidth>
        <InputLabel id={identifier}>{properties.name}</InputLabel>
        <Select
          labelId={identifier}
          value={value}
          label={properties.name}
          onChange={handleChange}
        >
          {properties.choices.map((choice, i) => (
            <MenuItem value={i} key={i}>
              {choice}
            </MenuItem>
          ))}
        </Select>
      </FormControl>
    </Box>
  );
}

const sayHello = Juce.getNativeFunction("sayHello");

const SpectrumDataReceiver_eventId = "spectrumData";

function interpolate(a, b, s) {
  let result = new Array(a.length).fill(0);

  for (const [i, val] of a.entries()) result[i] += (1 - s) * val;

  for (const [i, val] of b.entries()) result[i] += s * val;

  return result;
}

function mod(dividend, divisor) {
  const quotient = Math.floor(dividend / divisor);
  return dividend - divisor * quotient;
}

class SpectrumDataReceiver {
  constructor(bufferLength) {
    this.bufferLength = bufferLength;
    this.buffer = new Array(this.bufferLength);
    this.readIndex = 0;
    this.writeIndex = 0;
    this.lastTimeStampMs = 0;
    this.timeResolutionMs = 0;

    let self = this;
    this.spectrumDataRegistrationId = window.__JUCE__.backend.addEventListener(
      SpectrumDataReceiver_eventId,
      () => {
        fetch(Juce.getBackendResourceAddress("spectrumData.json"))
          .then((response) => response.text())
          .then((text) => {
            const data = JSON.parse(text);

            if (self.timeResolutionMs == 0) {
              self.timeResolutionMs = data.timeResolutionMs;

              // We want to stay behind the write index by a full batch plus one
              // so that we can keep reading buffered frames until we receive the
              // new batch
              self.readIndex = -data.frames.length - 1;

              self.buffer.fill(new Array(data.frames[0].length).fill(0));
            }

            for (const f of data.frames)
              self.buffer[mod(self.writeIndex++, self.bufferLength)] = f;
          });
      }
    );
  }

  getBufferItem(index) {
    return this.buffer[mod(index, this.buffer.length)];
  }

  getLevels(timeStampMs) {
    if (this.timeResolutionMs == 0) return null;

    const previousTimeStampMs = this.lastTimeStampMs;
    this.lastTimeStampMs = timeStampMs;

    if (previousTimeStampMs == 0) return this.buffer[0];

    const timeAdvance =
      (timeStampMs - previousTimeStampMs) / this.timeResolutionMs;
    this.readIndex += timeAdvance;

    const integralPart = Math.floor(this.readIndex);
    const fractionalPart = this.readIndex - integralPart;

    return interpolate(
      this.getBufferItem(integralPart),
      this.getBufferItem(integralPart + 1),
      fractionalPart
    );
  }

  unregister() {
    window.__JUCE__.backend.removeEventListener(
      this.spectrumDataRegistrationId
    );
  }
}

function FreqBandInfo() {
  const canvasRef = useRef(null);
  let dataReceiver = null;
  let isActive = true;

  // eslint-disable-next-line no-unused-vars
  const render = (timeStampMs) => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext("2d");
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    var grd = ctx.createLinearGradient(0, 0, 0, canvas.height);
    grd.addColorStop(0, "#1976d2");
    grd.addColorStop(1, "#dae9f8");
    ctx.fillStyle = grd;

    if (dataReceiver != null) {
      const levels = dataReceiver.getLevels(timeStampMs);

      if (levels != null) {
        const numBars = levels.length;
        const barWidth = canvas.width / numBars;
        const barHeight = canvas.height;

        for (const [i, l] of levels.entries()) {
          ctx.fillRect(
            i * barWidth,
            barHeight - l * barHeight,
            barWidth,
            l * barHeight
          );
        }
      }
    }

    if (isActive) window.requestAnimationFrame(render);
  };

  useEffect(() => {
    dataReceiver = new SpectrumDataReceiver(10);
    isActive = true;
    window.requestAnimationFrame(render);

    return function cleanup() {
      isActive = false;
      dataReceiver.unregister();
    };
  });

  const canvasStyle = {
    marginLeft: "0",
    marginRight: "0",
    marginTop: "1em",
    display: "block",
    width: "94%",
    bottom: "0",
    position: "absolute",
  };

  return (
    <Box>
      <canvas height={90} style={canvasStyle} ref={canvasRef}></canvas>
    </Box>
  );
}

function App() {
  const controlParameterIndexUpdater = new Juce.ControlParameterIndexUpdater(
    controlParameterIndexAnnotation
  );

  document.addEventListener("mousemove", (event) => {
    controlParameterIndexUpdater.handleMouseMove(event);
  });

  const [open, setOpen] = useState(false);
  const [snackbarMessage, setMessage] = useState("No message received yet");

  const openSnackbar = () => {
    setOpen(true);
  };

  const handleClose = (event, reason) => {
    if (reason === "clickaway") {
      return;
    }

    setOpen(false);
  };

  const action = (
    <>
      <IconButton
        size="small"
        aria-label="close"
        color="inherit"
        onClick={handleClose}
      >
        <CloseIcon fontSize="small" />
      </IconButton>
    </>
  );

  return (
    <div>
      <Container>
        <JuceSlider identifier="cutoffSlider" title="Cutoff" />
      </Container>
      <CardActions style={{ justifyContent: "center" }}>
        <Button
          variant="contained"
          sx={{ marginTop: 2 }}
          onClick={() => {
            sayHello("JUCE").then((result) => {
              setMessage(result);
              openSnackbar();
            });
          }}
        >
          Call backend function
        </Button>
      </CardActions>
      <CardActions style={{ justifyContent: "center" }}>
        <Button
          variant="contained"
          sx={{ marginTop: 2 }}
          onClick={() => {
            fetch(Juce.getBackendResourceAddress("data.txt"))
              .then((response) => response.text())
              .then((text) => {
                setMessage("Data fetched: " + text);
                openSnackbar();
              });
          }}
        >
          Fetch data from backend
        </Button>
      </CardActions>
      <JuceCheckbox identifier="muteToggle" />
      <br></br>
      <JuceComboBox identifier="filterTypeCombo" />
      <FreqBandInfo></FreqBandInfo>
      <Snackbar
        open={open}
        autoHideDuration={6000}
        onClose={handleClose}
        message={snackbarMessage}
        action={action}
      />
    </div>
  );
}

export default App;
